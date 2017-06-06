#include "File_list_widget.h"
#include "../model/File_data.h"
#include "../model/File_meta.h"
#include "../model/Session.h"
#include "../model/Submission.h"

#include <Wt/Dbo/Transaction>
#include <Wt/Http/Request>
#include <Wt/Http/Response>
#include <Wt/WAnchor>
#include <Wt/WLink>
#include <Wt/WPushButton>
#include <Wt/WText>

// A resource for downloading a source file.
class File_resource : public Wt::WResource
{
public:
    File_resource(const Wt::Dbo::ptr<File_meta>&, Wt::WObject* parent);
    ~File_resource();

    void handleRequest(const Wt::Http::Request&, Wt::Http::Response&) override;

private:
    Wt::Dbo::ptr<File_meta> source_file_;
};

// Action for deleting a source file.
class File_deleter : public Wt::WObject
{
public:
    File_deleter(const Wt::Dbo::ptr<File_meta>& source_file,
                 Wt::Signal<>& changed,
                 Session& session)
            : changed_(changed),
              source_file_(source_file),
              session_(session) {}

    void go();

private:
    Wt::Signal<>& changed_;
    Wt::Dbo::ptr<File_meta> source_file_;
    Session& session_;
};

File_list_widget::File_list_widget(const Wt::Dbo::ptr<Submission>& submission,
                                   Session& session,
                                   Wt::WContainerWidget* parent)
        : WTable(parent),
          submission_(submission),
          session_(session)
{
    setStyleClass("file-list");
    changed_.connect(this, &File_list_widget::reload);
    reload();
}

void File_list_widget::reload()
{
    clear();
    deleters_.clear();

    Wt::Dbo::Transaction transaction(session_);

    bool can_delete = submission_->can_submit(session_.user());

    new Wt::WText("<strong>filename</strong>", elementAt(0, 0));
    new Wt::WText("<strong>loc</strong>", elementAt(0, 1));
    if (can_delete)
        new Wt::WText("<strong>rm</strong>", elementAt(0, 2));

    int row = 1;

    for (Wt::Dbo::ptr<File_meta> file : submission_->source_files_sorted()) {
        Wt::WResource  * download = new File_resource(file, this);
        Wt::WAnchor    * anchor   = new Wt::WAnchor(Wt::WLink(download),
                                                    file->name(),
                                                    elementAt(row, 0));
        Wt::WText      * loc      = new Wt::WText(
                boost::lexical_cast<std::string>(file->line_count()),
                elementAt(row, 1));

        elementAt(row, 1)->setStyleClass("file-list-loc");

        anchor->setToolTip("download");
        loc->setToolTip("lines of code");

        if (can_delete) {
            auto remove = new Wt::WPushButton("X", elementAt(row, 2));
            remove->setToolTip("delete file");

            auto deleter = std::make_unique<File_deleter>(
                    file, changed_, session_);
            remove->clicked().connect(&*deleter, &File_deleter::go);
            deleters_.push_back(std::move(deleter));
        }

        ++row;
    }
}

File_resource::File_resource(const Wt::Dbo::ptr<File_meta>& source_file,
                             Wt::WObject* parent)
        : WResource(parent),
          source_file_(source_file)
{
    suggestFileName(source_file_->name());
}

File_resource::~File_resource()
{
    beingDeleted();
}

void File_resource::handleRequest(const Wt::Http::Request& request,
                                  Wt::Http::Response& response)
{
    Wt::Dbo::Transaction transaction(*source_file_.session());
    Wt::Dbo::ptr<File_data> text = source_file_->file_data().lock();
    transaction.commit();

    response.setMimeType("text/plain");
    response.out() << text->contents();
}

void File_deleter::go()
{
    Wt::Dbo::Transaction transaction(session_);
    if (source_file_->submission()->can_submit(session_.user())) {
        source_file_->submission().modify()->touch();
        source_file_.remove();
        transaction.commit();

        changed_.emit();
    }
}


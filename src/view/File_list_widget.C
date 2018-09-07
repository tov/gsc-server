#include "File_list_widget.h"
#include "../model/File_data.h"
#include "../model/File_meta.h"
#include "../Session.h"
#include "../model/Submission.h"

#include <Wt/Dbo/Transaction.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WAnchor.h>
#include <Wt/WLink.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

// A resource for downloading a source file.
class File_resource : public Wt::WResource
{
public:
    explicit File_resource(const Wt::Dbo::ptr<File_meta>&);
    ~File_resource();

    void handleRequest(const Wt::Http::Request&, Wt::Http::Response&) override;

private:
    Wt::Dbo::ptr<File_meta> source_file_;
};

File_list_widget::File_list_widget(const Wt::Dbo::ptr<Submission>& submission,
                                   Session& session)
        : submission_(submission),
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

    elementAt(0, 0)->addNew<Wt::WText>("<strong>filename</strong>");
    elementAt(0, 1)->addNew<Wt::WText>("<strong>loc</strong>");
    elementAt(0, 2)->addNew<Wt::WText>("<strong>bytes</strong>");
    if (can_delete)
        elementAt(0, 3)->addNew<Wt::WText>("<strong>rm</strong>");

    int row = 1;

    for (Wt::Dbo::ptr<File_meta> file : submission_->source_files_sorted()) {
        auto download = std::make_shared<File_resource>(file);

        auto anchor = elementAt(row, 0)->addNew<Wt::WAnchor>(
                Wt::WLink(download), file->name());
        auto loc = elementAt(row, 1)->addNew<Wt::WText>(
                boost::lexical_cast<std::string>(file->line_count()));
        auto bytes = elementAt(row, 2)->addNew<Wt::WText>(
                boost::lexical_cast<std::string>(file->byte_count()));

        elementAt(row, 1)->setStyleClass("file-list-loc");
        elementAt(row, 2)->setStyleClass("file-list-loc");

        anchor->setToolTip("download");
        loc->setToolTip("lines of code");
        bytes->setToolTip("file size in bytes");

        if (can_delete) {
            auto remove = elementAt(row, 3)->addNew<Wt::WPushButton>("X");
            remove->setToolTip("delete file");

            auto deleter = std::make_unique<File_deleter>(file, changed_, session_);
            remove->clicked().connect(&*deleter, &File_deleter::go);
            deleters_.push_back(std::move(deleter));
        }

        ++row;
    }
}

File_resource::File_resource(const Wt::Dbo::ptr<File_meta>& source_file)
        : source_file_(source_file)
{
    suggestFileName(source_file_->name(),
                    Wt::ContentDisposition::Inline);
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

File_deleter::File_deleter(const Wt::Dbo::ptr<File_meta>& source_file,
                           Wt::Signal<>& changed,
                           Session& session)
        : changed_(changed),
          source_file_(source_file),
          session_(session)
{}

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


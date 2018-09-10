#include "File_list_widget.h"
#include "../Session.h"
#include "../model/File_data.h"
#include "../model/File_meta.h"
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

// Action for deleting a source file.
class File_deleter : public Wt::WObject
{
public:
    File_deleter(const Wt::Dbo::ptr<File_meta>& source_file,
                 Wt::Signal<>* changed,
                 Session& session);

    void go();

private:
    Wt::Signal<>* changed_;
    Wt::Dbo::ptr<File_meta> source_file_;
    Session& session_;
};

File_list_widget::File_list_widget(const Wt::Dbo::ptr<Submission>& submission,
                                   Session& session,
                                   Wt::Signal<>* changed)
        : submission_(submission),
          session_(session),
          changed_(changed)
{
    setStyleClass("file-list");
    reload();
}

void File_list_widget::reload()
{
    clear();

    Wt::Dbo::Transaction transaction(session_);

    auto const source_files = submission_->source_files_sorted();

    if (source_files.empty()) return;

    bool can_delete = submission_->can_submit(session_.user());

    elementAt(0, 0)->addNew<Wt::WText>("<strong>filename</strong>");
    elementAt(0, 1)->addNew<Wt::WText>("<strong>type</strong>");
    elementAt(0, 2)->addNew<Wt::WText>("<strong>bytes</strong>");
    if (can_delete)
        elementAt(0, 3)->addNew<Wt::WText>("<strong>rm</strong>");

    int row = 1;

    for (auto const& file : source_files) {
        auto download = std::make_shared<File_resource>(file);

        auto anchor = elementAt(row, 0)->addNew<Wt::WAnchor>(
                Wt::WLink(download), file->name());
        auto type = elementAt(row, 1)->addNew<Wt::WText>(
                stringify(file->purpose()));
        auto bytes = elementAt(row, 2)->addNew<Wt::WText>(
                boost::lexical_cast<std::string>(file->byte_count()));

        elementAt(row, 1)->setStyleClass("file-list-type");
        elementAt(row, 2)->setStyleClass("file-list-bytes");

        anchor->setToolTip("view or download");
        type->setToolTip("file purpose");
        bytes->setToolTip("file size in bytes");

        if (can_delete) {
            auto remove = elementAt(row, 3)->addNew<Wt::WPushButton>("X");
            remove->setToolTip("delete file");

            auto deleter = remove->addChild(
                    std::make_unique<File_deleter>(file, changed_, session_));
            remove->clicked().connect(deleter, &File_deleter::go);
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

    response.setMimeType(source_file_->media_type());
    text->contents().write(response.out());
}

File_deleter::File_deleter(const Wt::Dbo::ptr<File_meta>& source_file,
                           Wt::Signal<>* changed,
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

        if (changed_) changed_->emit();
    }
}


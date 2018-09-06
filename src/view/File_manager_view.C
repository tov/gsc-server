#include "File_manager_view.h"
#include "File_list_widget.h"
#include "File_viewer_widget.h"
#include "Submission_owner_widget.h"
#include "../model/Session.h"
#include "../model/File_data.h"
#include "../model/File_meta.h"
#include "../model/Submission.h"

#include <Wt/Http/Response.h>
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WFileUpload.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WMessageBox.h>
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>

#include <boost/filesystem.hpp>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <vector>

class Date_list : public Wt::WTable
{
public:
    Date_list(const Wt::Dbo::ptr<Submission>&,
              Wt::WContainerWidget* parent = nullptr);

    void reload();

private:
    Wt::Dbo::ptr<Submission> submission_;
    Wt::WText* due_date_;
    Wt::WText* eval_date_;
    Wt::WText* last_modified_;

    static const Wt::WString date_format_;
    static Wt::WString format_(const Wt::WDateTime&);
};

class File_uploader : public Wt::WContainerWidget
{
public:
    File_uploader(const Wt::Dbo::ptr<Submission>&,
                  Wt::Signal<>& changed,
                  Session& session,
                  WContainerWidget* parent);

private:
    Wt::Signal<>& changed_;
    Wt::Dbo::ptr<Submission> submission_;
    Session& session_;

    Wt::WFileUpload* upload_;

    void uploaded_();
    void too_large_();
};

const Wt::WString Date_list::date_format_ = "ddd, MMM d 'at' h:mm AP";

Date_list::Date_list(const Wt::Dbo::ptr<Submission>& submission,
                     Wt::WContainerWidget* parent)
        : WTable(parent), submission_(submission)
{
    setStyleClass("date-list");

    new Wt::WText("Due:", elementAt(0, 0));
    new Wt::WText("Eval due:", elementAt(1, 0));
    new Wt::WText("Last modified:", elementAt(2, 0));

    due_date_ = new Wt::WText("", elementAt(0, 1));
    eval_date_ = new Wt::WText("", elementAt(1, 1));
    last_modified_ = new Wt::WText("", elementAt(2, 1));

    reload();
}

void Date_list::reload()
{
    due_date_->setText(format_(submission_->effective_due_date()));
    eval_date_->setText(format_(submission_->effective_eval_date()));
    last_modified_->setText(format_(submission_->last_modified()));
}

Wt::WString Date_list::format_(const Wt::WDateTime& date)
{
    return date.toLocalTime().toString(date_format_);
}

File_uploader::File_uploader(const Wt::Dbo::ptr<Submission>& submission,
                             Wt::Signal<>& changed, Session& session,
                             WContainerWidget* parent)
        : WContainerWidget(parent),
          submission_(submission),
          changed_(changed),
          session_(session)
{
    upload_ = new Wt::WFileUpload(this);
    upload_->setFileTextSize(100);
    upload_->setMultiple(true);
    upload_->uploaded().connect(this, &File_uploader::uploaded_);
    upload_->fileTooLarge().connect(this, &File_uploader::too_large_);

    if (Wt::WApplication::instance()->environment().ajax()) {
        setStyleClass("file-uploader btn");
        auto label = new Wt::WText("Upload files...", this);
        upload_->changed().connect(upload_, &Wt::WFileUpload::upload);
    } else {
        auto button = new Wt::WPushButton("Upload", this);
        button->clicked().connect(upload_, &Wt::WFileUpload::upload);
    }
}

void File_uploader::uploaded_()
{
    if (!submission_->can_submit(session_.user()))
        return;

    for (auto& file : upload_->uploadedFiles()) {
        std::ifstream spool(file.spoolFileName());
        std::string   contents;

        spool.seekg(0, std::ios::end);
        contents.reserve((size_t) spool.tellg());
        spool.seekg(0, std::ios::beg);

        contents.assign(std::istreambuf_iterator<char>(spool),
                        std::istreambuf_iterator<char>());

        boost::filesystem::path filename(file.clientFileName());

        File_meta::upload(filename.filename().string(),
                          contents, submission_);
    }

    changed_.emit();
}

void File_uploader::too_large_()
{
    auto message_box = new Wt::WMessageBox("Upload Error",
                                           "File too large",
                                           Wt::Critical, Wt::Ok,
                                           this);
    message_box->setModal(true);
    message_box->buttonClicked().connect(std::bind([=]() {
        delete message_box;
    }));
    message_box->show();
}

File_manager_view::File_manager_view(
        const Wt::Dbo::ptr<Submission>& submission,
        Session& session,
        Wt::WContainerWidget* parent) :

        Abstract_file_view(submission, session, parent)
{
    new Submission_owner_widget(submission_, session_, right_column_);

    auto file_list = new File_list_widget(submission_, session_);

    if (submission_->can_submit(session_.user()))
        new File_uploader(submission_, file_list->changed(),
                          session_, right_column_);

    right_column_->addWidget(file_list);

    auto date_list = new Date_list(submission_, right_column_);

    file_list->changed().connect(viewer_, &File_viewer_widget::reload);
    file_list->changed().connect(date_list, &Date_list::reload);
}

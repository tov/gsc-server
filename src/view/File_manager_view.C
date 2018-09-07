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

class Quota_display : public Wt::WTemplate
{
public:
    Quota_display(const Wt::Dbo::ptr<Submission>&);

    void reload();

private:
    Wt::Dbo::ptr<Submission> submission_;
    Wt::WText* field_;
};

class Date_list : public Wt::WTable
{
public:
    explicit Date_list(const Wt::Dbo::ptr<Submission>&);

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
                  Wt::Signal<>& changed, Session& session);

private:
    Wt::Signal<>& changed_;
    Wt::Dbo::ptr<Submission> submission_;
    Session& session_;

    Wt::WFileUpload* upload_;

    void uploaded_();
    void too_large_();
};

const std::string quota_display_template =
        "<p class=\"quota-display\">"
          "File quota remaining: ${bytes} bytes"
        "</p>";

Quota_display::Quota_display(const Wt::Dbo::ptr<Submission>& submission)
        : Wt::WTemplate(quota_display_template)
        , submission_{submission}
{
    field_ = bindWidget("bytes", std::make_unique<Wt::WText>());
    reload();
}

void Quota_display::reload()
{
    field_->setText(boost::lexical_cast<std::string>(
            submission_->remaining_space()));
}

const Wt::WString Date_list::date_format_ = "ddd, MMM d 'at' h:mm AP";

Date_list::Date_list(const Wt::Dbo::ptr<Submission>& submission)
        : submission_(submission)
{
    setStyleClass("date-list");

    elementAt(0, 0)->addNew<Wt::WText>("Due:");
    elementAt(1, 0)->addNew<Wt::WText>("Eval due:");
    elementAt(2, 0)->addNew<Wt::WText>("Last modified:");

    due_date_ = elementAt(0, 1)->addNew<Wt::WText>();
    eval_date_ = elementAt(1, 1)->addNew<Wt::WText>();
    last_modified_ = elementAt(2, 1)->addNew<Wt::WText>();

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
                             Wt::Signal<>& changed, Session& session)
        : submission_(submission),
          changed_(changed),
          session_(session)
{
    upload_ = addNew<Wt::WFileUpload>();
    upload_->setFileTextSize(100);
    upload_->setMultiple(true);
    upload_->uploaded().connect(this, &File_uploader::uploaded_);
    upload_->fileTooLarge().connect(this, &File_uploader::too_large_);

    if (Wt::WApplication::instance()->environment().ajax()) {
        setStyleClass("file-uploader btn");
        auto label = addNew<Wt::WText>("Upload files...");
        upload_->changed().connect(upload_, &Wt::WFileUpload::upload);
    } else {
        auto button = addNew<Wt::WPushButton>("Upload");
        button->clicked().connect(upload_, &Wt::WFileUpload::upload);
    }
}

void File_uploader::uploaded_()
{
    if (!submission_->can_submit(session_.user()))
        return;

    dbo::Transaction transaction(session_);

    for (auto& file : upload_->uploadedFiles()) {
        std::ifstream spool(file.spoolFileName());
        boost::filesystem::path filename(file.clientFileName());

        spool.seekg(0, std::ios::end);
        int file_size = spool.tellg();

        auto notify = [&](std::string message) {
            auto message_box = new Wt::WMessageBox("File Quota Exceeded",
                                                   message,
                                                   Wt::Icon::Critical,
                                                   Wt::StandardButton::Ok);
            message_box->setModal(true);
            message_box->buttonClicked().connect(std::bind([=]() {
                delete message_box;
            }));
            message_box->show();
        };

        if (file_size > File_meta::max_byte_count) {
            std::ostringstream msg;
            msg << "File “" << filename
                    << "” cannot be uploaded because it is " << file_size
                    << " bytes, which exceeds the per-file quota of "
                    << File_meta::max_byte_count << " bytes.";
            notify(msg.str());
            break;
        }

        if (! submission_->has_sufficient_space(file_size,
                                                filename.filename().string())) {
            std::ostringstream msg;
            msg << "File “" << filename
                    << "” cannot be uploaded because its size of "
                    << file_size << " bytes exceeds your remaining quota of "
                    << submission_->remaining_space() << " bytes.";
            notify(msg.str());
            break;
        }

        std::string contents;
        contents.reserve((size_t) file_size);
        spool.seekg(0, std::ios::beg);

        contents.assign(std::istreambuf_iterator<char>(spool),
                        std::istreambuf_iterator<char>());

        File_meta::upload(filename.filename().string(),
                          contents, submission_);
    }

    changed_.emit();
}

void File_uploader::too_large_()
{
    auto message_box = new Wt::WMessageBox(
            "Upload Error", "File too large",
            Wt::Icon::Critical, Wt::StandardButton::Ok);
    message_box->setModal(true);
    message_box->buttonClicked().connect(std::bind([=]() {
        delete message_box;
    }));
    message_box->show();
}

File_manager_view::File_manager_view(const Wt::Dbo::ptr<Submission>& submission,
                                     Session& session)
        : Abstract_file_view(submission, session)
{
    right_column_->addNew<Submission_owner_widget>(submission_, session_);

    auto file_list = std::make_unique<File_list_widget>(submission_, session_);

    if (submission_->can_submit(session_.user()))
        right_column_->addNew<File_uploader>(submission_,
                                             file_list->changed(),
                                             session_);

    auto raw_file_list = right_column_->addWidget(std::move(file_list));
    auto quota_display = right_column_->addNew<Quota_display>(submission);
    auto raw_date_list = right_column_->addNew<Date_list>(submission_);

    raw_file_list->changed().connect(viewer_, &File_viewer_widget::reload);
    raw_file_list->changed().connect(quota_display, &Quota_display::reload);
    raw_file_list->changed().connect(raw_date_list, &Date_list::reload);
}

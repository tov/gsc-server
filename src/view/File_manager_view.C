#include "File_manager_view.h"
#include "File_list_widget.h"
#include "File_viewer_widget.h"
#include "Notification.h"
#include "Submission_owner_widget.h"
#include "../Session.h"
#include "../model/Assignment.h"
#include "../model/File_data.h"
#include "../model/File_meta.h"
#include "../model/Submission.h"
#include "../common/format.h"

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

class Quota_display : public Wt::WTemplate, public Reloadable
{
public:
    Quota_display(const Wt::Dbo::ptr<Submission>&);

    void reload() override;

private:
    Wt::Dbo::ptr<Submission> submission_;
    Wt::WText* field_;
};

class Date_list : public Wt::WTable, public Reloadable
{
public:
    explicit Date_list(const Wt::Dbo::ptr<Submission>&);

    void reload() override;

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
    Wt::WContainerWidget* progress_holder_;

    void reset_();
    void start_upload_();
    void uploaded_();
    void too_large_(long size);
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
    submission_.reread();
    field_->setText(with_commas(submission_->remaining_space()));
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
    reset_();
}

void File_uploader::reset_()
{
    clear();

    progress_holder_ = addNew<Wt::WContainerWidget>();
    progress_holder_->setStyleClass("progress-holder");
    progress_holder_->hide();
    auto progress_bar = progress_holder_->addNew<Wt::WProgressBar>();

    upload_ = addNew<Wt::WFileUpload>();
    upload_->setProgressBar(progress_bar);
    upload_->setFileTextSize(100);
    upload_->setMultiple(true);
    upload_->changed().connect([=] { start_upload_(); });
    upload_->uploaded().connect([=] { uploaded_(); });
    upload_->fileTooLarge().connect(this, &File_uploader::too_large_);

    if (Wt::WApplication::instance()->environment().ajax()) {
        setStyleClass("file-uploader btn");
        auto label = addNew<Wt::WText>("Upload files...");
        upload_->changed().connect([=] { start_upload_(); });
    } else {
        auto button = addNew<Wt::WPushButton>("Upload");
        button->clicked().connect([=] { start_upload_(); });
    }
}

void File_uploader::start_upload_()
{
    progress_holder_->show();
    upload_->upload();
}

void File_uploader::uploaded_()
{
    dbo::Transaction transaction(session_);

    if (!submission_->can_submit(session_.user()))
        return;

    for (auto& file : upload_->uploadedFiles()) {
        std::ifstream spool(file.spoolFileName());
        boost::filesystem::path filepath(file.clientFileName());
        std::string filename = filepath.filename().string();

        spool.seekg(0, std::ios::end);
        int file_size = spool.tellg();
        spool.seekg(0, std::ios::beg);

        if (file_size > File_meta::max_byte_count) {
            Notification("File Too Large", parent())
                    << "File ‘" << filename << "’ cannot be uploaded because it is "
                    << with_commas(file_size) << " bytes, which exceeds the per-file quota of "
                    << with_commas(File_meta::max_byte_count) << " bytes.";
            break;
        }

        if (! submission_->has_sufficient_space(file_size, filename)) {
            Notification("File Quota Exceeded", parent())
                    << "File ‘" << filename << "’ cannot be uploaded because its size of "
                    << with_commas(file_size) << " bytes exceeds your remaining quota of "
                    << with_commas(submission_->remaining_space()) << " bytes.";
            break;
        }

        Bytes contents{spool, file_size};

        File_meta::upload(filename, contents, submission_, session_.user());
    }

    changed_.emit();
    reset_();
}

void File_uploader::too_large_(long size)
{
    auto max_size = Wt::WApplication::instance()->maximumRequestSize();
    Notification("Files Too Large", parent())
            << "Files cannot be uploaded because their total size ("
            << with_commas(size)
            << " bytes) exceeds the per-upload quota of "
            << with_commas(max_size)
            << " bytes.";
    reset_();
}

File_manager_view::File_manager_view(const Wt::Dbo::ptr<Submission>& submission,
                                     Session& session)
        : Abstract_file_view(submission, session)
{
    bool can_modify =
            submission_->can_submit(session_.user()) &&
            submission_->assignment()->web_allowed();

    auto submission_owner =
            right_column_->addNew<Submission_owner_widget>(
                    submission_, session_, changed_);

    auto file_list = std::make_unique<File_list_widget>(
            submission_, can_modify, session_, &changed_);

    if (can_modify)
        right_column_->addNew<File_uploader>(submission_, changed_, session_);

    file_list_         = right_column_->addWidget(std::move(file_list));
    quota_display_     = right_column_->addNew<Quota_display>(submission);
    date_list_         = right_column_->addNew<Date_list>(submission_);
}

void File_manager_view::reload()
{
    dbo::Transaction transaction(session_);
    Abstract_file_view::reload();
    date_list_->reload();
    file_list_->reload();
    quota_display_->reload();
}

#include "../../../Session.h"
#include "../../../common/format.h"
#include "../../../model/Assignment.h"
#include "../../../model/File_data.h"
#include "../../../model/File_meta.h"
#include "../../../model/Submission.h"
#include "../Notification.h"
#include "../widget/File_list_widget.h"
#include "../widget/File_viewer.h"
#include "../widget/Submission_owner_widget.h"
#include "File_manager_view.h"

#include <Wt/Http/Response.h>
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WFileUpload.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

#ifdef GSC_USE_FILESYSTEM
#include <filesystem>
#endif // GSC_USE_FILESYSTEM

#include <fstream>
#include <streambuf>
#include <vector>

class Quota_display : public Wt::WTemplate, public Reloadable {
public:
  Quota_display(Submission_context &);

  void reload() override;

private:
  Wt::WText *field_;
  Submission_context &context_;
};

class Date_list : public Wt::WTable, public Reloadable {
public:
  Date_list(Submission_context &);

  void reload() override;

private:
  Submission_context &context_;

  Wt::WText *due_date_;
  Wt::WText *eval_date_;
  Wt::WText *last_modified_;

  static const Wt::WString date_format_;
  static Wt::WString format_(const Wt::WDateTime &);
};

class File_uploader : public Wt::WContainerWidget {
public:
  File_uploader(Submission_context &);

private:
  Submission_context &context_;

  Wt::WFileUpload *upload_;
  Wt::WContainerWidget *progress_holder_;

  void reset_();
  void start_upload_();
  void uploaded_();
  void too_large_(long size);
};

const std::string quota_display_template =
    "<p class=\"quota-display\">"
    "File quota remaining: ${bytes} bytes"
    "</p>";

Quota_display::Quota_display(Submission_context &context)
    : Wt::WTemplate(quota_display_template), context_{context} {
  field_ = bindNew<Wt::WText>("bytes");
}

void Quota_display::reload() {
  field_->setText(with_commas(context_.submission()->remaining_space()));
}

const Wt::WString Date_list::date_format_ = "ddd, MMM d 'at' h:mm AP";

Date_list::Date_list(Submission_context &context) : context_{context} {
  setStyleClass("date-list");

  elementAt(0, 0)->addNew<Wt::WText>("Code due:");
  elementAt(1, 0)->addNew<Wt::WText>("Self-eval due:");
  elementAt(2, 0)->addNew<Wt::WText>("Last modified:");

  due_date_ = elementAt(0, 1)->addNew<Wt::WText>();
  eval_date_ = elementAt(1, 1)->addNew<Wt::WText>();
  last_modified_ = elementAt(2, 1)->addNew<Wt::WText>();
}

void Date_list::reload() {
  auto &submission = context_.submission();
  due_date_->setText(format_(submission->effective_due_date()));
  eval_date_->setText(format_(submission->effective_eval_date()));
  last_modified_->setText(format_(submission->last_modified()));
}

Wt::WString Date_list::format_(const Wt::WDateTime &date) {
  return date.toLocalTime().toString(date_format_);
}

File_uploader::File_uploader(Submission_context &context) : context_{context} {
  reset_();
}

void File_uploader::reset_() {
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
    setStyleClass("file-uploader btn btn-success");
    addNew<Wt::WText>("Upload files...");
    upload_->changed().connect([=] { start_upload_(); });
  } else {
    setStyleClass("backup-file-uploader");
    auto button = addNew<Wt::WPushButton>("Upload");
    button->setStyleClass("btn btn-success");
    button->clicked().connect([=] { start_upload_(); });
  }
}

void File_uploader::start_upload_() {
  progress_holder_->show();
  upload_->upload();
}

void File_uploader::uploaded_() {
  auto &session = context_.session();
  auto &submission = context_.submission();

  dbo::Transaction transaction(session);

  if (!submission->can_submit(session.user()))
    return;

  try {
    for (auto &file : upload_->uploadedFiles()) {
      std::ifstream spool(file.spoolFileName());

      std::string const &client_file_name = file.clientFileName();
#ifdef GSC_USE_FILESYSTEM
      std::filesystem::path filepath(client_file_name);
      std::string filename = filepath.filename().string();
#else  // GSC_USE_FILESYSTEM
      size_t slash_pos = client_file_name.rfind('/');
      size_t start_pos = slash_pos == std::string::npos ? 0 : slash_pos + 1;
      std::string filename = client_file_name.substr(start_pos);
#endif // GSC_USE_FILESYSTEM

      spool.seekg(0, std::ios::end);
      int file_size = spool.tellg();
      spool.seekg(0, std::ios::beg);

      Bytes contents{spool, file_size};
      File_meta::upload(filename, contents, submission, session.user());
    }
  } catch (Html_error const &exn) {
    auto note = Notification(exn.title());
    exn.write_body_html(note);
  }

  context_.notify();
  reset_();
}

void File_uploader::too_large_(long size) {
  auto max_size = Wt::WApplication::instance()->maximumRequestSize();
  Notification("Files Too Large")
      << "Files cannot be uploaded because their total size ("
      << with_commas(size) << " bytes) exceeds the per-upload limit of "
      << with_commas(max_size) << " bytes.";
  reset_();
}

File_manager_view::File_manager_view(const Wt::Dbo::ptr<Submission> &submission,
                                     Session &session)
    : Abstract_file_view(submission, session) {
  auto user = session.user();
  bool can_submit = submission->can_submit(user);
  bool can_web = submission->assignment()->web_allowed() || user->can_admin();
  bool can_modify = can_submit && can_web;

  right_column_->addNew<Submission_owner_widget>(submission, session,
                                                 changed());

  auto file_list = std::make_unique<File_list_widget>(submission, can_modify,
                                                      session, changed());

  if (can_modify)
    right_column_->addNew<File_uploader>(context());
  else if (can_submit)
    right_column_->addNew<Wt::WText>(
        "You must use the command-line <tt>gsc</tt> client "
        "to modify this submission.");

  file_list_ = right_column_->addWidget(std::move(file_list));
#ifdef GSC_SHOW_QUOTA
  quota_display_ = right_column_->addNew<Quota_display>(context());
#endif // GSC_SHOW_QUOTA
  date_list_ = right_column_->addNew<Date_list>(context());

  on_change_();
}

void File_manager_view::on_change() { on_change_(); }

void File_manager_view::on_change_() {
  dbo::Transaction transaction(session());
#ifdef GSC_SHOW_QUOTA
  quota_display_->reload();
#endif // GSC_SHOW_QUOTA
  date_list_->reload();
}

#include "../../../Session.h"
#include "../../../common/util.h"
#include "../../../model/Assignment.h"
#include "../../../model/Exam_grade.h"
#include "../../../model/Submission.h"
#include "../../../model/auth/User.h"
#include "../widget/Date_time_edit.h"
#include "../widget/Partner_notification_widget.h"
#include "Submissions_view.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/Transaction.h>
#include <Wt/WApplication.h>
#include <Wt/WDateTime.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

Submissions_view::Model::Item::Item(dbo::ptr<Submission> const &submission,
                                    dbo::ptr<User> const &principal)
    : submission(submission), file_count(submission->file_count()),
      eval_status(submission->eval_status()), principal(principal) {}

Submissions_view::Model::Model(dbo::ptr<User> const &principal,
                               Session &session)
    : principal(principal) {
  dbo::Transaction transaction(session);

  for (const auto &submission : principal->submissions()) {
    // Make sure these are loaded now.
    submission->user1()->id();
    if (submission->user2())
      submission->user2()->id();

    submissions[submission->assignment()->number()] =
        Item(submission, principal);
  }

  for (auto exam_grade : Exam_grade::find_by_user(principal)) {
    exams.push_back(exam_grade);
  }
}

Submissions_view::Row_view::Row_view(const Row_model &model, Session &session,
                                     WTableRow *row)
    : model_(model), session_(session), row_(row) {
  row->elementAt(NAME)->addNew<WText>(model_.submission->assignment()->name());
  status_ = row_->elementAt(STATUS)->addNew<WText>();
  grade_ = row_->elementAt(GRADE)->addNew<WText>();
  action_ = row_->elementAt(ACTION)->addNew<WPushButton>();

  row->elementAt(GRADE)->setStyleClass("numeric");

  action_->clicked().connect(this, &Submissions_view::Row_view::action);
}

void Submissions_view::Row_view::add_headings(WTableRow *row) {
  row->elementAt(NAME)->addNew<WText>("Homework");
  row->elementAt(STATUS)->addNew<WText>("Status");
  row->elementAt(DUE_DATE)->addNew<WText>("Code Due");
  row->elementAt(EVAL_DATE)->addNew<WText>("Self-Eval Due");
  row->elementAt(GRADE)->addNew<WText>("Score");
  row->elementAt(ACTION)->addNew<WText>("Action");
}

void Submissions_view::Row_view::set_files_action(const char *title) {
  action_->setText(title);
  action_url_ = model_.submission->url_for_user(model_.principal, false);
}

void Submissions_view::Row_view::set_eval_action(const char *title) {
  action_->setText(title);
  action_url_ = model_.submission->url_for_user(model_.principal, true);
}

void Submissions_view::Row_view::set_action_style_class(const char *style) {
  action_->setStyleClass(style);
}

void Submissions_view::Row_view::update() {
  auto const now = WDateTime::currentDateTime();

  auto time_to = [&](const WDateTime &date) {
    return now.timeTo(date, chrono::seconds{2});
  };

  action_->show();

  WString status;
  switch (model_.submission->status()) {
  case Submission::Status::future:
    row_->setStyleClass("future");
    status += "Opens in ";
    status += time_to(model_.submission->assignment()->open_date());
    action_->hide();
    break;

  case Submission::Status::open:
  case Submission::Status::extended:
    row_->setStyleClass("open");
    if (model_.submission->assignment()->web_allowed()) {
      set_files_action("Submit");
    } else {
      set_files_action("Browse");
    }

    if (model_.file_count == 0) {
      status += "Due in ";
      status += time_to(model_.submission->effective_due_date());
      set_action_style_class("btn btn-primary");
    } else {
      status += "Submitted ";
      status += to_string(model_.file_count);
      status += " file";
      if (model_.file_count > 1)
        status += "s";
      set_action_style_class("btn btn-info");
    }
    break;

  case Submission::Status::self_eval:
  case Submission::Status::extended_eval:
    switch (model_.eval_status) {
    case Submission::Eval_status::empty: {
      row_->setStyleClass("self-eval needed");
      status += "Self-eval due in ";
      status += time_to(model_.submission->effective_eval_date());
      set_eval_action("Start");
      set_action_style_class("btn btn-primary");
      break;
    }

    case Submission::Eval_status::started: {
      row_->setStyleClass("self-eval started");
      status += "Self-eval due in ";
      status += time_to(model_.submission->effective_eval_date());
      set_eval_action("Continue");
      set_action_style_class("btn btn-primary");
      break;
    }

    case Submission::Eval_status::complete: {
      status += "Self-eval complete";
      set_eval_action("Edit");
      set_action_style_class("btn btn-success");
      break;
    }

    case Submission::Eval_status::overdue:
      goto closed;
      // Can't happen if the eval status is only overdue
      // when the submission status is closed.
    }
    break;

  closed:
  case Submission::Status::closed: {
    row_->setStyleClass("closed");
    status += "Closed ";
    status += model_.submission->effective_eval_date().timeTo(
        now, chrono::seconds{2});
    status += " ago";
    set_eval_action("View");
    set_action_style_class("btn btn-link");
    break;
  }
  }

  status_->setText(status);

  row_->elementAt(DUE_DATE)->setStyleClass(
      model_.submission->effective_due_date() < now ? "past" : "");
  row_->elementAt(EVAL_DATE)->setStyleClass(
      model_.submission->effective_eval_date() < now ? "past" : "");

  dbo::Transaction transaction(session_);
  if (model_.submission->is_graded()) {
    grade_->setText(model_.submission->grade_string());
  }
}

void Submissions_view::Row_view::action() {
  if (!action_url_.empty())
    WApplication::instance()->setInternalPath(action_url_, true);
}

class Student_submissions_view_row : public Submissions_view::Row_view {
public:
  Student_submissions_view_row(const Row_model &model, Session &session,
                               WTableRow *row);
};

namespace {

[[maybe_unused]] bool is_eod(WTime const &time) {
  return time.hour() == 23 && time.minute() == 59;
}

WString friendly_due_date(Wt::WDateTime const &date_time) {
  auto local_time = date_time.toLocalTime();
  return local_time.toString();

  // if (is_eod(local_time.time())) {
  //    return local_time.toString("dddd d MMM (Z)");
  //} else {
  //}
}

} // end anonymous namespace

Student_submissions_view_row::Student_submissions_view_row(
    const Row_model &model, Session &session, WTableRow *row)
    : Submissions_view::Row_view(model, session, row) {
  row_->elementAt(DUE_DATE)->addNew<WText>(
      friendly_due_date(model_.submission->effective_due_date()));
  row_->elementAt(EVAL_DATE)->addNew<WText>(
      friendly_due_date(model_.submission->effective_eval_date()));
}

class Admin_submissions_view_row : public Submissions_view::Row_view {
public:
  Admin_submissions_view_row(const Row_model &model, Session &session,
                             WTableRow *row);

  virtual void update() override;

protected:
  virtual void set_files_action(const char *) override;
  virtual void set_eval_action(const char *) override;
  virtual void set_action_style_class(const char *) override;

  Date_time_edit *due_date_;
  Date_time_edit *eval_date_;

  void due_date_changed_();
  void eval_date_changed_();
};

Admin_submissions_view_row::Admin_submissions_view_row(const Row_model &model,
                                                       Session &session,
                                                       WTableRow *row)
    : Submissions_view::Row_view(model, session, row) {
  due_date_ = row->elementAt(DUE_DATE)->addNew<Date_time_edit>();
  eval_date_ = row->elementAt(EVAL_DATE)->addNew<Date_time_edit>();

  due_date_->set_date_format("MM/dd/yy");
  eval_date_->set_date_format("MM/dd/yy");

  due_date_->set_time_format("HH:mm");
  eval_date_->set_time_format("HH:mm");

  due_date_->changed().connect(this,
                               &Admin_submissions_view_row::due_date_changed_);
  eval_date_->changed().connect(
      this, &Admin_submissions_view_row::eval_date_changed_);
}

void Admin_submissions_view_row::update() {
  Submissions_view::Row_view::update();
  due_date_->set_date_time(model_.submission->effective_due_date());
  eval_date_->set_date_time(model_.submission->effective_eval_date());

  if (model_.submission->extended()) {
    due_date_->setStyleClass("extended");
    auto date =
        model_.submission->assignment()->due_date().toLocalTime().toString();
    due_date_->setToolTip("Extended from " + date);
  } else {
    due_date_->setStyleClass("");
    due_date_->setToolTip("");
  }

  if (model_.submission->eval_extended()) {
    eval_date_->setStyleClass("extended");
    auto date =
        model_.submission->assignment()->eval_date().toLocalTime().toString();
    eval_date_->setToolTip("Extended from " + date);
  } else {
    eval_date_->setStyleClass("");
    eval_date_->setToolTip("");
  }
}

void Admin_submissions_view_row::due_date_changed_() {
  if (due_date_->validate() == ValidationState::Valid) {
    due_date_->setStyleClass("");
    dbo::Transaction transaction(session_);
    model_.submission.modify()->set_due_date(due_date_->date_time());
    transaction.commit();
    update();
  } else {
    due_date_->setStyleClass("invalid");
  }
}

void Admin_submissions_view_row::eval_date_changed_() {
  if (eval_date_->validate() == ValidationState::Valid) {
    eval_date_->setStyleClass("");
    dbo::Transaction transaction(session_);
    model_.submission.modify()->set_eval_date(eval_date_->date_time());
    transaction.commit();
    update();
  } else {
    eval_date_->setStyleClass("invalid");
  }
}

void Admin_submissions_view_row::set_files_action(const char *) {
  Submissions_view::Row_view::set_files_action("Files");
}

void Admin_submissions_view_row::set_eval_action(const char *) {
  Submissions_view::Row_view::set_eval_action("Eval");
}

void Admin_submissions_view_row::set_action_style_class(const char *) {
  Submissions_view::Row_view::set_action_style_class("btn");
}

unique_ptr<Submissions_view::Row_view>
Submissions_view::Row_view::construct(const Row_model &model, Session &session,
                                      WTableRow *row) {
  if (session.user()->can_admin()) {
    auto result = make_unique<Admin_submissions_view_row>(model, session, row);
    result->update();
    return move(result);
  } else {
    auto result =
        make_unique<Student_submissions_view_row>(model, session, row);
    result->update();
    return move(result);
  }
}

Submissions_view::Submissions_view(const dbo::ptr<User> &user, Session &session)
    : session_(session), model_{user, session} {
  setStyleClass("submissions-view");
  changed_.connect(this, &Submissions_view::on_change_);
  reload_();
}

void Submissions_view::reload_() {
  clear();

  addNew<Partner_notification_widget>(model_.principal, dbo::ptr<Submission>{},
                                      session_, changed_);

  auto table = addNew<WTable>();
  table->setHeaderCount(1, Orientation::Horizontal);
  table->setHeaderCount(1, Orientation::Vertical);
  Row_view::add_headings(table->rowAt(0));

  int row = 1;
  for (const auto &each : model_.submissions) {
    if (!each.submission)
      continue;
    rows_.push_back(Submissions_view::Row_view::construct(each, session_,
                                                          table->rowAt(row++)));
  }

  auto exam_table = addNew<WTable>();
  exam_table->setStyleClass("exam-table");
  row = 0;
  for (const auto &each : model_.exams) {
    auto exam_row = exam_table->rowAt(row++);

    ostringstream fmt;
    fmt << "Exam " << each->number();
    exam_row->elementAt(0)->addNew<WText>(fmt.str());

    fmt.str("");
    fmt << each->points() << " / " << each->possible();
    exam_row->elementAt(1)->addNew<WText>(fmt.str());

    exam_row->elementAt(2)->addNew<WText>(each->pct_string());
  }
}

void Submissions_view::on_change_(Submission_change_message submission) {
  if (submission) {
    dbo::Transaction trans(session_);
    model_.submissions[submission->assignment_number()] =
        Model::Item(submission, model_.principal);
  }

  reload_();
}

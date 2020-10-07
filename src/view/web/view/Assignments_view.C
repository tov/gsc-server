#include "../../../Navigate.h"
#include "../../../model/Assignment.h"
#include "../Confirmation_dialog.h"
#include "../widget/Date_time_edit.h"
#include "Assignments_view.h"

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WCheckBox.h>
#include <Wt/WDateTime.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>
#include <Wt/WTime.h>

#include <sstream>

Assignments_view_row::Assignments_view_row(
    const dbo::ptr<Assignment> &assignment, Session &session,
    Wt::WTableRow *row)
    : assignment_(assignment), session_(session), row_(row) {
  row_->elementAt(NUMBER)->addNew<Wt::WText>(
      std::to_string(assignment_->number()));
  name_ = row_->elementAt(NAME)->addNew<Wt::WLineEdit>();
  partner_ = row_->elementAt(PARTNER)->addNew<Wt::WCheckBox>();
  web_allowed_ = row_->elementAt(WEB_ALLOWED)->addNew<Wt::WCheckBox>();
  open_date_ = row_->elementAt(OPEN_DATE)->addNew<Date_time_edit>();
  due_date_ = row_->elementAt(DUE_DATE)->addNew<Date_time_edit>();
  eval_date_ = row_->elementAt(EVAL_DATE)->addNew<Date_time_edit>();
  points_ = row_->elementAt(POINTS)->addNew<Wt::WLineEdit>();
  auto edit = row_->elementAt(ACTION)->addNew<Wt::WPushButton>("Edit");

  name_->setStyleClass("name-edit");
  points_->setStyleClass("points-edit");

  name_->changed().connect(std::bind([=]() {
    dbo::Transaction transaction(session_);
    assignment_.modify()->set_name(name_->text().toUTF8());
    transaction.commit();
    update_();
  }));

  partner_->changed().connect(std::bind([=]() {
    dbo::Transaction transaction(session_);
    assignment_.modify()->set_partner(partner_->isChecked());
    transaction.commit();
    update_();
  }));

  web_allowed_->changed().connect([=] {
    dbo::Transaction transaction(session_);
    assignment_.modify()->set_web_allowed(web_allowed_->isChecked());
    transaction.commit();
    update_();
  });

  points_->changed().connect(std::bind([=]() {
    std::string points_s = points_->text().toUTF8();
    int points = atoi(points_s.data());
    dbo::Transaction transaction(session_);
    assignment_.modify()->set_points(points);
    transaction.commit();
    update_();
  }));

  open_date_->set_date_format("MM/dd/yy");
  due_date_->set_date_format("MM/dd/yy");
  eval_date_->set_date_format("MM/dd/yy");

  open_date_->set_time_format("HH:mm");
  due_date_->set_time_format("HH:mm");
  eval_date_->set_time_format("HH:mm");

  open_date_->changed().connect(std::bind([=]() {
    if (open_date_->validate() == Wt::ValidationState::Valid) {
      dbo::Transaction transaction(session_);
      assignment_.modify()->set_open_date(open_date_->date_time());
      transaction.commit();
      open_date_->setStyleClass("");
      update_();
    } else {
      open_date_->setStyleClass("invalid");
    }
  }));

  due_date_->changed().connect(std::bind([=]() {
    if (due_date_->validate() == Wt::ValidationState::Valid) {
      dbo::Transaction transaction(session_);
      assignment_.modify()->set_due_date(due_date_->date_time());
      transaction.commit();
      due_date_->setStyleClass("");
      update_();
    } else {
      due_date_->setStyleClass("invalid");
    }
  }));

  eval_date_->changed().connect(std::bind([=]() {
    if (eval_date_->validate() == Wt::ValidationState::Valid) {
      dbo::Transaction transaction(session_);
      assignment_.modify()->set_eval_date(eval_date_->date_time());
      transaction.commit();
      eval_date_->setStyleClass("");
      update_();
    } else {
      due_date_->setStyleClass("invalid");
    }
  }));

  std::ostringstream url_stream;
  url_stream << "/hw/" << assignment->number();
  auto url = url_stream.str();
  //    edit->clicked().connect(Navigate(url.str()));
  edit->clicked().connect([=]() { Navigate::to(url); });

  update_();
}

void Assignments_view_row::add_headings(Wt::WTableRow *row) {
  row->elementAt(NUMBER)->addNew<Wt::WText>("#");
  row->elementAt(NAME)->addNew<Wt::WText>("Title");
  row->elementAt(PARTNER)->addNew<Wt::WText>("2")->setToolTip("Allow partners");
  row->elementAt(WEB_ALLOWED)
      ->addNew<Wt::WText>("W")
      ->setToolTip("Allow web submission");
  row->elementAt(OPEN_DATE)->addNew<Wt::WText>("Opens");
  row->elementAt(DUE_DATE)->addNew<Wt::WText>("Code Due");
  row->elementAt(EVAL_DATE)->addNew<Wt::WText>("Self-Eval Due");
  row->elementAt(POINTS)->addNew<Wt::WText>("Pts");
  row->elementAt(ACTION)->addNew<Wt::WText>("Action");
}

void Assignments_view_row::update_() const {
  name_->setText(assignment_->name());
  partner_->setChecked(assignment_->partner());
  web_allowed_->setChecked(assignment_->web_allowed());
  open_date_->set_date_time(assignment_->open_date());
  due_date_->set_date_time(assignment_->due_date());
  eval_date_->set_date_time(assignment_->eval_date());
  points_->setText(std::to_string(assignment_->points()));

  //    open_date_->set_top(due_date_->date_time());
  //    due_date_->set_bottom(open_date_->date_time());
  //    due_date_->set_top(eval_date_->date_time());
  //    eval_date_->set_bottom(due_date_->date_time());
}

Assignments_view::Assignments_view(Session &session) : session_(session) {
  setStyleClass("assignments-view");

  dbo::Transaction transaction(session_);
  auto assignments = session_.find<Assignment>().orderBy("number").resultList();

  table_ = addNew<Wt::WTable>();
  table_->setHeaderCount(1, Wt::Orientation::Vertical);
  table_->setHeaderCount(1, Wt::Orientation::Horizontal);
  Assignments_view_row::add_headings(table_->rowAt(0));

  int row = 1;
  for (const auto &assignment : assignments) {
    rows_.push_back(std::make_unique<Assignments_view_row>(
        assignment, session_, table_->rowAt(row++)));
  }
  transaction.commit();

  auto buttons = addNew<Wt::WContainerWidget>();
  buttons->setStyleClass("buttons");

  auto more = buttons->addNew<Wt::WPushButton>("More");
  auto fewer = buttons->addNew<Wt::WPushButton>("Fewer");

  more->clicked().connect(this, &Assignments_view::more_);
  fewer->clicked().connect(this, &Assignments_view::fewer_);
}

void Assignments_view::more_() {
  int number = (int)rows_.size() + 1;
  std::string name = "Homework " + std::to_string(number);
  auto local_date = Wt::WLocalDateTime::currentDateTime();
  local_date.setTime(Wt::WTime(23, 59));
  auto date = local_date.toUTC();

  dbo::Transaction transaction(session_);
  auto assignment =
      session_.addNew<Assignment>(number, name, 0, date, date, date);
  transaction.commit();

  rows_.push_back(std::make_unique<Assignments_view_row>(
      assignment, session_, table_->rowAt(table_->rowCount())));
}

void Assignments_view::fewer_() {
  if (rows_.empty())
    return;

  auto assignment = rows_.back()->assignment_;

  dbo::Transaction transaction(session_);
  auto submission_count = assignment->submissions().size();
  auto eval_item_count = assignment->eval_items().size();
  transaction.commit();

  if (submission_count == 0 && eval_item_count == 0) {
    real_fewer_();
  } else {
    std::ostringstream message;
    message << assignment->name() << " currently has " << submission_count
            << " submission(s) and " << eval_item_count
            << " eval item(s). Are you sure you want "
               "to delete it?";

    Confirmation_dialog::create(message.str())
        .accepted()
        .connect(this, &Assignments_view::real_fewer_);
  }
}

void Assignments_view::real_fewer_() {
  if (rows_.empty())
    return;

  dbo::Transaction transaction(session_);
  rows_.back()->assignment_.remove();
  transaction.commit();

  table_->removeRow(table_->rowCount() - 1);
  rows_.pop_back();
}

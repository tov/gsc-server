#include "../../../common/format.h"
#include "../Confirmation_dialog.h"
#include "../Notification.h"
#include "Partner_notification_widget.h"
#include "Submission_owner_widget.h"

#include "../../../Session.h"
#include "../../../model/Assignment.h"
#include "../../../model/Partner_request.h"
#include "../../../model/Submission.h"
#include "../../../model/auth/User.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include <sstream>

Submission_owner_widget::Submission_owner_widget(
    Submission_context const &context)
    : Submission_context{context} {
  load_();
}

Submission_owner_widget::Submission_owner_widget(
    Wt::Dbo::ptr<Submission> const &submission, Session &session,
    Submission_change_signal &changed)
    : Submission_context(changed, session, submission) {
  load_();
}

void Submission_owner_widget::load_() {
  impl_ = setNewImplementation<Wt::WContainerWidget>();
  setStyleClass("submission-owner-widget");
  reload_();
}

void Submission_owner_widget::reload_() {
  impl_->clear();

  switch (session().user()->role()) {
  case User::Role::Admin:
    update_admin_();
    break;

  case User::Role::Grader:
    update_grader_();
    break;

  case User::Role::Student:
    update_student_();
    break;
  }
}

void Submission_owner_widget::update_admin_() {
  update_grader_();

  dbo::Transaction transaction(session().dbo());
  auto status = submission()->status();
  transaction.commit();

  if (submission()->user2() && (status == Submission::Status::open ||
                                status == Submission::Status::extended)) {
    impl_->addNew<Wt::WText>(" ");
    auto button = impl_->addNew<Wt::WPushButton>("X");
    button->setStyleClass("btn btn-danger");
    button->setToolTip("Break up partnership");
    button->clicked().connect([=]() {
      Confirmation_dialog::create(
          "Are you sure you want to break up this partnership? "
          "Their submission will be split, and they may have "
          "to resubmit.")
          .accepted()
          .connect([=]() { break_up_partnership_(); });
    });
  }

  else if (!submission()->user2() && submission()->assignment()->partner()) {
    impl_->addNew<Wt::WBreak>();
    impl_->addNew<Wt::WText>("Partner with: ");
    auto edit = impl_->addNew<Wt::WLineEdit>();
    edit->setStyleClass("username");
    edit->setPlaceholderText("(make it so)");
    edit->enterPressed().connect([=]() {
      html_try<Notification>([=] {
        auto guard = dbo::Transaction(session());

        if (auto user2 = User::find_by_name(session(), edit->text().toUTF8())) {
          auto joined =
              Partner_request::confirm(session(), submission()->user1(), user2,
                                       submission()->assignment());
          notify(joined);
        } else {
          edit->setText("");
        }
      });
    });
  }
}

void Submission_owner_widget::update_grader_() {
  std::ostringstream message;

  dbo::Transaction transaction(session());
  message << "Submitted by <strong>" << submission()->user1()->name()
          << "</strong>";

  if (submission()->user2()) {
    message << " and <strong>" << submission()->user2()->name() << "</strong>";
  }
  transaction.commit();

  impl_->addNew<Wt::WText>(message.str());
}

void Submission_owner_widget::update_student_() {
  dbo::ptr<User> self = session().user();

  dbo::Transaction transaction(session());

  if (submission()->user2()) {
    auto other_user = self == submission()->user1() ? submission()->user2()
                                                    : submission()->user1();

    std::ostringstream message;
    message << "Partnered with <strong>" << other_user->name() << "</strong>";
    impl_->addNew<Wt::WText>(message.str());
    return;
  }

  impl_->addNew<Partner_notification_widget>(self, submission(), session(),
                                             changed());
}

void Submission_owner_widget::break_up_partnership_() {
  html_try<Notification>([=] {
    auto guard = dbo::Transaction(session());
    submission().modify()->divorce();
    notify();
  });
}

void Submission_owner_widget::on_change() { reload_(); }

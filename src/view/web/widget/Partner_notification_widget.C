#include "../../../Session.h"
#include "../../../common/util.h"
#include "../../../model/Assignment.h"
#include "../../../model/Partner_request.h"
#include "../../../model/Submission.h"
#include "../../../model/auth/User.h"
#include "../Notification.h"
#include "Partner_notification_widget.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WApplication.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include <sstream>

class Partner_requestor_widget : public Wt::WContainerWidget {
public:
  explicit Partner_requestor_widget(Submission_context &);

private:
  Submission_context &context_;
  Wt::WLineEdit *edit_;

  void submit_();
  void error_(std::string const &message);
};

class Partner_pending_widget : public Wt::WContainerWidget {
public:
  Partner_pending_widget(Submission_context &,
                         const Wt::Dbo::ptr<Partner_request> &,
                         bool inline_buttons = false);

private:
  Submission_context &context_;
  Wt::Dbo::ptr<Partner_request> request_;

  void cancel_();
};

class Partner_confirmer_widget : public Wt::WContainerWidget {
public:
  Partner_confirmer_widget(Submission_context &main,
                           const Wt::Dbo::ptr<Partner_request> &,
                           bool inline_buttons = false);

private:
  Submission_context &context_;
  Wt::Dbo::ptr<Partner_request> request_;

  void accept_();
  void reject_();
};

Partner_pending_widget::Partner_pending_widget(
    Submission_context &context, const dbo::ptr<Partner_request> &request,
    bool inline_buttons)
    : context_(context), request_(request) {
  setStyleClass("partner-notification partner-pending");

  ostringstream message;
  message << "Your partner request to <strong>" << request_->requestee()->name()
          << "</strong>";
  if (!context_.submission())
    message << " for " << request_->assignment()->name();
  message << " is pending confirmation. ";
  addNew<WText>(message.str());

  auto buttons = addNew<WContainerWidget>();
  buttons->setInline(inline_buttons);
  buttons->setStyleClass(inline_buttons ? "buttons-inline" : "buttons");

  auto cancel = buttons->addNew<WPushButton>("Cancel");
  cancel->clicked().connect(this, &Partner_pending_widget::cancel_);
}

void Partner_pending_widget::cancel_() {
  dbo::Transaction transaction(context_.session());
  request_.remove();
  transaction.commit();
  context_.notify();
}

Partner_confirmer_widget::Partner_confirmer_widget(
    Submission_context &context, const dbo::ptr<Partner_request> &request,
    bool inline_buttons)
    : context_(context), request_(request) {
  setStyleClass("partner-notification partner-confirmer");

  ostringstream message;
  message << "You have a partner request from <strong>"
          << request->requestor()->name() << "</strong>";
  if (!context_.submission())
    message << " for " << request_->assignment()->name();
  message << ".";

  addNew<WText>(message.str());

  auto buttons = addNew<WContainerWidget>();
  buttons->setStyleClass(inline_buttons ? "buttons-inline" : "buttons");
  buttons->setInline(inline_buttons);
  auto reject = buttons->addNew<WPushButton>("Reject");
  auto accept = buttons->addNew<WPushButton>("Accept");

  reject->clicked().connect(this, &Partner_confirmer_widget::reject_);
  accept->clicked().connect(this, &Partner_confirmer_widget::accept_);
}

void Partner_confirmer_widget::accept_() {
  try {
    dbo::Transaction transaction(context_.session());
    auto joint_submission = request_->confirm(context_.session());
    context_.notify(joint_submission);
  } catch (Html_error const &exn) {
    auto note = Notification(exn.title());
    exn.write_body_html(note);
  }
}

void Partner_confirmer_widget::reject_() {
  dbo::Transaction transaction(context_.session());
  request_.remove();
  transaction.commit();
  removeFromParent();
}

Partner_requestor_widget::Partner_requestor_widget(Submission_context &context)
    : context_(context) {
  setStyleClass("partner-notification partner-requestor");

  addNew<WText>("Request partner: ");

  edit_ = addNew<WLineEdit>();
  edit_->setStyleClass("username");
  edit_->setPlaceholderText("(ask them)");

  auto buttons = addNew<WContainerWidget>();
  buttons->setStyleClass("buttons");
  auto request = buttons->addNew<WPushButton>("Send");
  request->setStyleClass("btn btn-primary");

  request->clicked().connect(this, &Partner_requestor_widget::submit_);
  edit_->enterPressed().connect(this, &Partner_requestor_widget::submit_);
}

void Partner_requestor_widget::submit_() {
  if (edit_->text().empty())
    return;

  try {
    ostringstream message;

    dbo::Transaction transaction(context_.session());
    auto user2 = User::find_by_name(context_.session(), edit_->text().toUTF8());
    if (!user2) {
      message << "User ‘" << edit_->text() << "’ does not exist.";
      error_(message.str());
      return;
    }

    if (auto incoming = Partner_request::find_by_requestor_and_assignment(
            context_.session(), user2, context_.assignment());
        incoming && incoming->requestee() == context_.principal()) {
      if (auto joint_submission = incoming->confirm(context_.session())) {
        context_.notify(joint_submission);
        return;
      }
    }

    auto request =
        Partner_request::create(context_.session(), context_.principal(), user2,
                                context_.assignment(), message);
    transaction.commit();

    if (request)
      context_.notify();
    else
      error_(message.str());

  } catch (Html_error const &exn) {
    auto note = Notification(exn.title());
    exn.write_body_html(note);
  }
}

void Partner_requestor_widget::error_(string const &message) {
  Notification("Error").and_then([=] { edit_->setFocus(true); }) << message;
}

Partner_notification_widget::Partner_notification_widget(
    const dbo::ptr<User> &principal, const dbo::ptr<Submission> &submission,
    Session &session, Submission_change_signal &signal)
    : Submission_context{signal, session, submission, principal} {
  impl_ = setNewImplementation<WContainerWidget>();
  reload_();
}

void Partner_notification_widget::reload_() {
  dbo::Transaction transaction(session());

  impl_->clear();

  if (submission()) {
    auto incoming = Partner_request::find_by_requestee_and_assignment(
        session(), principal(), assignment());
    for (const auto &each : incoming)
      if (each->is_active(session()))
        impl_->addNew<Partner_confirmer_widget>(context(), each, false);

    auto outgoing = Partner_request::find_by_requestor_and_assignment(
        session(), principal(), assignment());
    if (outgoing && outgoing->is_active(session()))
      impl_->addNew<Partner_pending_widget>(context(), outgoing, false);
    else if (submission()->can_submit(session().user()) &&
             assignment()->partner())
      impl_->addNew<Partner_requestor_widget>(context());
  } else {
    auto incoming = Partner_request::find_by_requestee(session(), principal());
    for (const auto &each : incoming)
      if (each->is_active(session()))
        impl_->addNew<Partner_confirmer_widget>(context(), each, true);

    auto outgoing = Partner_request::find_by_requestor(session(), principal());
    for (const auto &each : outgoing)
      if (each->is_active(session()))
        impl_->addNew<Partner_pending_widget>(context(), each, true);
  }
}

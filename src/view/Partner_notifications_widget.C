#include "Partner_notifications_widget.h"
#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../model/Session.h"
#include "../model/Partner_request.h"
#include "../model/Submission.h"

Partner_pending_widget::Partner_pending_widget(
        Partner_notification_widget* main,
        const Wt::Dbo::ptr<Partner_request>& request,
        bool inline_buttons)
        : main_(main)
        , request_(request)
        , WContainerWidget(main->impl_)
{
    setStyleClass("partner-notification partner-pending");

    std::ostringstream message;
    message << "Your partner request to <strong>"
            << request_->requestee()->name()
            << "</strong>";
    if (!main_->submission_)
        message << " for " << request_->assignment()->name();
    message << " is pending confirmation. ";
    new Wt::WText(message.str(), this);

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setInline(inline_buttons);
    buttons->setStyleClass(inline_buttons? "buttons-inline" : "buttons");

    auto cancel = new Wt::WPushButton("Cancel", buttons);
    cancel->clicked().connect(this, &Partner_pending_widget::cancel_);
}

void Partner_pending_widget::cancel_() {
    dbo::Transaction transaction(main_->session_);
    request_.remove();
    transaction.commit();
    main_->update_();
}

Partner_confirmer_widget::Partner_confirmer_widget(
        Partner_notification_widget* main,
        const Wt::Dbo::ptr<Partner_request>& request,
        bool inline_buttons)
        : main_(main)
        , request_(request)
        , WContainerWidget(main->impl_)
{
    setStyleClass("partner-notification partner-confirmer");

    std::ostringstream message;
    message << "You have a partner request from <strong>"
            << request->requestor()->name()
            << "</strong>";
    if (! main_->submission_)
        message << " for " << request_->assignment()->name();
    message << ".";

    new Wt::WText(message.str(), this);

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass(inline_buttons? "buttons-inline" : "buttons");
    buttons->setInline(inline_buttons);
    auto reject = new Wt::WPushButton("Reject", buttons);
    auto accept = new Wt::WPushButton("Accept", buttons);

    reject->clicked().connect(this, &Partner_confirmer_widget::reject_);
    accept->clicked().connect(this, &Partner_confirmer_widget::accept_);
}

void Partner_confirmer_widget::accept_()
{
    dbo::Transaction transaction(main_->session_);
    auto submission = request_->confirm(main_->session_);
    transaction.commit();

    if (submission) {
        if (main_->submission_)
            Wt::WApplication::instance()->setInternalPath(submission->url(),
                                                          true);
        else
            main_->update_();
    } else {
        Wt::WMessageBox* box = new Wt::WMessageBox(
                "Error",
                Wt::WString::fromUTF8("That partner request has"
                                              " been withdrawn :("),
                Wt::Critical, Wt::Ok, this);
        box->setModal(true);
        box->buttonClicked().connect(std::bind([=] () {
            delete box;
            main_->update_();
        }));
        box->show(); }
}

void Partner_confirmer_widget::reject_()
{
    dbo::Transaction transaction(main_->session_);
    request_.remove();
    transaction.commit();
    delete this;
}

Partner_requestor_widget::Partner_requestor_widget
        (Partner_notification_widget* main)
        : WContainerWidget(main->impl_)
        , main_(main)
{
    setStyleClass("partner-notification partner-requestor");

    new Wt::WText("Request partner: ", this);

    edit_ = new Wt::WLineEdit(this);
    edit_->setStyleClass("username");
    edit_->setEmptyText("NetID");

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");
    auto request = new Wt::WPushButton("Send", buttons);

    request->clicked().connect(this, &Partner_requestor_widget::submit_);
    edit_->enterPressed().connect(this, &Partner_requestor_widget::submit_);
}

void Partner_requestor_widget::submit_()
{
    if (edit_->text().empty()) return;

    dbo::Transaction transaction(main_->session_);
    auto user2 = User::find_by_name(main_->session_, edit_->text().toUTF8());
    if (!user2 || user2->role() != User::Role::Student ||
        user2 == main_->session_.user())
    {
        error_();
        return;
    }

    auto request = Partner_request::create(main_->session_,
                                           main_->session_.user(),
                                           user2,
                                           main_->submission_->assignment());
    transaction.commit();

    if (request) main_->update_();
    else error_();
}

void Partner_requestor_widget::error_()
{
    std::ostringstream message;
    message << "User “" << edit_->text()
            << "” does not exist or is unavailable.";
    Wt::WMessageBox* box = new Wt::WMessageBox(
            "Error",
            Wt::WString::fromUTF8(message.str()),
            Wt::Critical, Wt::Ok, this);
    box->setModal(true);
    box->buttonClicked().connect(std::bind([=] () { delete box; }));
    box->show();
}

Partner_notification_widget::Partner_notification_widget(
        const Wt::Dbo::ptr<User>& user,
        const Wt::Dbo::ptr<Submission>& submission,
        Session& session,
        Wt::WContainerWidget* parent)
        : session_(session)
        , user_(user)
        , submission_(submission)
        , WCompositeWidget(parent)
{
    setImplementation(impl_ = new Wt::WContainerWidget());
    update_();
}

void Partner_notification_widget::update_() {
    impl_->clear();

    dbo::Transaction transaction(session_);

    if (submission_) {
        auto incoming = Partner_request::find_by_requestee_and_assignment(
                session_, user_, submission_->assignment());
        for (const auto& each : incoming)
            new Partner_confirmer_widget(this, each, false);

        if (auto outgoing = Partner_request::find_by_requestor_and_assignment(
                session_, user_, submission_->assignment()))
            new Partner_pending_widget(this, outgoing, false);
        else
            new Partner_requestor_widget(this);
    } else {
        auto incoming = Partner_request::find_by_requestee(session_, user_);
        for (const auto& each : incoming)
            new Partner_confirmer_widget(this, each, true);

        auto outgoing = Partner_request::find_by_requestor(session_, user_);
        for (const auto& each : outgoing)
            new Partner_pending_widget(this, each, true);
    }
}


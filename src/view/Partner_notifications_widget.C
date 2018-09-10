#include "Partner_notifications_widget.h"
#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../Session.h"
#include "../model/Partner_request.h"
#include "../model/Submission.h"
#include "../Navigate.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WApplication.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

Partner_pending_widget::Partner_pending_widget(
        Partner_notification_widget* main,
        const Wt::Dbo::ptr<Partner_request>& request,
        bool inline_buttons)
        : main_(main)
        , request_(request)
{
    setStyleClass("partner-notification partner-pending");

    std::ostringstream message;
    message << "Your partner request to <strong>"
            << request_->requestee()->name()
            << "</strong>";
    if (!main_->submission_)
        message << " for " << request_->assignment()->name();
    message << " is pending confirmation. ";
    addNew<Wt::WText>(message.str());

    auto buttons = addNew<Wt::WContainerWidget>();
    buttons->setInline(inline_buttons);
    buttons->setStyleClass(inline_buttons? "buttons-inline" : "buttons");

    auto cancel = buttons->addNew<Wt::WPushButton>("Cancel");
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
{
    setStyleClass("partner-notification partner-confirmer");

    std::ostringstream message;
    message << "You have a partner request from <strong>"
            << request->requestor()->name()
            << "</strong>";
    if (! main_->submission_)
        message << " for " << request_->assignment()->name();
    message << ".";

    addNew<Wt::WText>(message.str());

    auto buttons = addNew<Wt::WContainerWidget>();
    buttons->setStyleClass(inline_buttons? "buttons-inline" : "buttons");
    buttons->setInline(inline_buttons);
    auto reject = buttons->addNew<Wt::WPushButton>("Reject");
    auto accept = buttons->addNew<Wt::WPushButton>("Accept");

    reject->clicked().connect(this, &Partner_confirmer_widget::reject_);
    accept->clicked().connect(this, &Partner_confirmer_widget::accept_);
}

void Partner_confirmer_widget::accept_()
{
    dbo::Transaction transaction(main_->session_);
    auto joint_submission = request_->confirm(main_->session_);
    transaction.commit();

    if (joint_submission) {
        if (main_->submission_)
            Navigate::to(joint_submission->url());
        else
            main_->update_();
    } else {
        auto box = addNew<Wt::WMessageBox>(
                "Error",
                Wt::WString::fromUTF8("That partner request has"
                                              " been withdrawn :("),
                Wt::Icon::Critical, Wt::StandardButton::Ok);
        box->setModal(true);
        box->buttonClicked().connect(std::bind([=] () {
            delete box;
            main_->update_();
        }));
        box->show();
    }
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
        : main_(main)
{
    setStyleClass("partner-notification partner-requestor");

    addNew<Wt::WText>("Request partner: ");

    edit_ = addNew<Wt::WLineEdit>();
    edit_->setStyleClass("username");
    edit_->setPlaceholderText("NetID");

    auto buttons = addNew<Wt::WContainerWidget>();
    buttons->setStyleClass("buttons");
    auto request = buttons->addNew<Wt::WPushButton>("Send");

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
    auto box = addNew<Wt::WMessageBox>(
            "Error",
            Wt::WString::fromUTF8(message.str()),
            Wt::Icon::Critical, Wt::StandardButton::Ok);
    box->setModal(true);
    box->buttonClicked().connect(std::bind([=] () { box->removeFromParent(); }));
    box->show();
}

Partner_notification_widget::Partner_notification_widget(
            const Wt::Dbo::ptr<User>& user,
            const Wt::Dbo::ptr<Submission>& submission,
            Session& session)
        : session_(session)
        , user_(user)
        , submission_(submission)
{
    impl_ = setNewImplementation<Wt::WContainerWidget>();
    update_();
}

void Partner_notification_widget::update_() {
    impl_->clear();

    dbo::Transaction transaction(session_);

    if (submission_) {
        auto incoming = Partner_request::find_by_requestee_and_assignment(
                session_, user_, submission_->assignment());
        for (const auto& each : incoming)
            if (each->is_active(session_))
                impl_->addNew<Partner_confirmer_widget>(this, each, false);

        auto outgoing = Partner_request::find_by_requestor_and_assignment(
                session_, user_, submission_->assignment());
        if (outgoing && outgoing->is_active(session_))
            impl_->addNew<Partner_pending_widget>(this, outgoing, false);
        else
            if (submission_->can_submit(user_) &&
                    submission_->assignment()->partner())
                impl_->addNew<Partner_requestor_widget>(this);
    } else {
        auto incoming = Partner_request::find_by_requestee(session_, user_);
        for (const auto& each : incoming)
            if (each->is_active(session_))
                impl_->addNew<Partner_confirmer_widget>(this, each, true);

        auto outgoing = Partner_request::find_by_requestor(session_, user_);
        for (const auto& each : outgoing)
            if (each->is_active(session_))
                impl_->addNew<Partner_pending_widget>(this, each, true);
    }
}


#include "Submission_owner_widget.h"
#include "User_suggester.h"
#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../model/Session.h"
#include "../model/Submission.h"
#include "../model/Partner_request.h"

#include <Wt/Dbo/Dbo>
#include <Wt/WLineEdit>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WText>

#include <sstream>

class Partner_pending : public Wt::WContainerWidget
{
public:
    Partner_pending(Submission_owner_widget* main,
                    const dbo::ptr<Partner_request>& request,
                    Wt::WContainerWidget* parent = nullptr);

private:
    dbo::ptr<Partner_request> request_;
    Submission_owner_widget* main_;

    void cancel_();
};

Partner_pending::Partner_pending(Submission_owner_widget* main,
                                 const dbo::ptr<Partner_request>& request,
                                 Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          main_(main),
          request_(request)
{
    setStyleClass("partner-notification partner-pending");

    std::ostringstream message;
    message << "Your partner request to <strong>"
            << request_->requestee()->name()
            << "</strong> is pending confirmation. ";
    new Wt::WText(message.str(), this);

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");

    auto cancel = new Wt::WPushButton("Cancel", buttons);
    cancel->clicked().connect(this, &Partner_pending::cancel_);
}

void Partner_pending::cancel_() {
    dbo::Transaction transaction(main_->session_);
    request_.remove();
    transaction.commit();
    main_->update_();
}

class Partner_requestor : public Wt::WContainerWidget
{
public:
    Partner_requestor(Submission_owner_widget* main,
                      Wt::WContainerWidget* parent = nullptr);

private:
    Submission_owner_widget* main_;
    Wt::WLineEdit* edit_;

    void submit_();
    void error_();
};

Partner_requestor::Partner_requestor(Submission_owner_widget* main,
                                     Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          main_(main)
{
    setStyleClass("partner-notification partner-requestor");

    new Wt::WText("Request partner: ", this);

    edit_ = new Wt::WLineEdit(this);
    edit_->setStyleClass("username");
    edit_->setEmptyText("NetID");

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");
    auto request = new Wt::WPushButton("Send", buttons);

    request->clicked().connect(this, &Partner_requestor::submit_);
    edit_->enterPressed().connect(this, &Partner_requestor::submit_);
}

void Partner_requestor::submit_()
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

void Partner_requestor::error_()
{
    std::ostringstream message;
    message << "User “" << edit_->text()
            << "” does not exist or is unavailable.";
    Wt::WMessageBox* box = new Wt::WMessageBox(
            "Error",
            Wt::WString::fromUTF8(message.str()),
            Wt::Critical, Wt::Ok, this);
    box->setModal(true);
    box->buttonClicked().connect(std::bind([=] () {
        delete box;
//        main_->update_();
    }));
    box->show();
}

class Partner_confirmer : public Wt::WContainerWidget
{
public:
    Partner_confirmer(Submission_owner_widget* main,
                      const dbo::ptr<Partner_request>& request,
                      Wt::WContainerWidget* parent = nullptr);

private:
    Submission_owner_widget* main_;
    dbo::ptr<Partner_request> request_;

    void accept_();
    void reject_();
};

Partner_confirmer::Partner_confirmer(Submission_owner_widget* main,
                                     const dbo::ptr<Partner_request>& request,
                                     Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          main_(main),
          request_(request)
{
    setStyleClass("partner-notification partner-confirmer");

    std::ostringstream message;
    message << "You have a partner request from <strong>"
            << request->requestor()->name()
            << "</strong>. ";
    new Wt::WText(message.str(), this);

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");
    auto reject = new Wt::WPushButton("Reject", buttons);
    auto accept = new Wt::WPushButton("Accept", buttons);

    reject->clicked().connect(this, &Partner_confirmer::reject_);
    accept->clicked().connect(this, &Partner_confirmer::accept_);
}

void Partner_confirmer::accept_()
{
    dbo::Transaction transaction(main_->session_);
    auto submission = request_->confirm(main_->session_);
    transaction.commit();

    if (submission) {
        Wt::WApplication::instance()->setInternalPath(submission->url(), true);
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
        box->show();
    }
}

void Partner_confirmer::reject_()
{
    dbo::Transaction transaction(main_->session_);
    request_.remove();
    transaction.commit();
    delete this;
}

Submission_owner_widget::Submission_owner_widget(
        const dbo::ptr<Submission>& submission,
        Session& session,
        Wt::WContainerWidget* parent)
        : WCompositeWidget(parent),
          session_(session),
          submission_(submission)
{
    setImplementation(impl_ = new Wt::WContainerWidget);
    setStyleClass("submission-owner-widget");
    update_();
}

void Submission_owner_widget::update_()
{
    impl_->clear();

    submission_.reread();

    switch (session_.user()->role()) {
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

void Submission_owner_widget::update_admin_()
{
    update_grader_();

    dbo::Transaction transaction(session_);
    auto status = submission_->status();
    transaction.commit();

    if (submission_->user2() &&
            (status == Submission::Status::open ||
             status == Submission::Status::extended))
    {
        new Wt::WText(" ", impl_);
        auto button = new Wt::WPushButton("x", impl_);
        button->clicked().connect(std::bind([=] () {
            dbo::Transaction transaction2(session_);
            submission_.modify()->set_user2(dbo::ptr<User>());
            transaction2.commit();

            update_();
        }));
    }
}

void Submission_owner_widget::update_grader_()
{
    std::ostringstream message;

    dbo::Transaction transaction(session_);
    message << "Submitted by <strong>"
            << submission_->user1()->name()
            << "</strong>";

    if (submission_->user2()) {
        message << " and <strong>"
                << submission_->user2()->name()
                << "</strong>";
    }
    transaction.commit();

    new Wt::WText(message.str(), impl_);
}

void Submission_owner_widget::update_student_()
{
    dbo::ptr<User> self = session_.user();

    dbo::Transaction transaction(session_);

    if (submission_->user2()) {
        auto other_user = self == submission_->user1()
                          ? submission_->user2()
                          : submission_->user1();

        std::ostringstream message;
        message << "Partnered with <strong>"
                << other_user->name()
                << "</strong>";
        new Wt::WText(message.str(), impl_);
        return;
    }

    auto status = submission_->status();
    if (!(status == Submission::Status::open ||
            status == Submission::Status::extended))
        return;

    auto assignment = submission_->assignment();
    if (!assignment->partner()) return;

    auto outgoing = Partner_request::find_by_requestor(session_, self,
                                                       assignment);
    if (outgoing) {
        new Partner_pending(this, outgoing, impl_);
    } else {
        new Partner_requestor(this, impl_);
    }

    auto incoming = Partner_request::find_by_requestee(session_, self,
                                                       assignment);
    for (const auto& request : incoming) {
        new Partner_confirmer(this, request, impl_);
    }
}

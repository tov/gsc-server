#include "Submission_owner_widget.h"
#include "User_suggester.h"
#include "Partner_notifications_widget.h"
#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../model/Session.h"
#include "../model/Submission.h"
#include "../model/Partner_request.h"

#include <Wt/Dbo/Dbo>
#include <Wt/WBreak>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>

#include <sstream>

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
        button->setToolTip("Break up partnership");
        button->clicked().connect(std::bind([=] () {
            dbo::Transaction transaction2(session_);
            submission_.modify()->set_user2(dbo::ptr<User>());
            transaction2.commit();

            update_();
        }));
    }

    else if (!submission_->user2()) {
        new Wt::WBreak(impl_);
        new Wt::WText("Partner with: ", impl_);
        auto edit = new Wt::WLineEdit(impl_);
        edit->setStyleClass("username");
        edit->setEmptyText("NetID");
        edit->enterPressed().connect(std::bind([=] () {
            dbo::Transaction transaction2(session_);

            auto user2 = User::find_by_name(session_, edit->text().toUTF8());
            if (!user2) return;

            auto other = Submission::find_by_assignment_and_user(
                    session_, submission_->assignment(), user2);
            if (!other) return;

            Submission::join_together(submission_, other);
            Partner_request::delete_requests(session_, submission_->user1(),
                                             submission_->assignment());
            Partner_request::delete_requests(session_, submission_->user2(),
                                             submission_->assignment());

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

    new Partner_notification_widget(self, submission_, session_, impl_);
}

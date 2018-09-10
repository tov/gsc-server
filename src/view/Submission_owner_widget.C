#include "Submission_owner_widget.h"
#include "Confirmation_dialog.h"
#include "User_suggester.h"
#include "Partner_notifications_widget.h"

#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../Session.h"
#include "../model/Submission.h"
#include "../model/Partner_request.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WBreak.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include <sstream>

Submission_owner_widget::Submission_owner_widget(const Wt::Dbo::ptr<Submission>& submission,
                                                 Session& session)
        : session_(session),
          submission_(submission)
{
    impl_ = setNewImplementation<Wt::WContainerWidget>();
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
        impl_->addNew<Wt::WText>(" ");
        auto button = impl_->addNew<Wt::WPushButton>("X");
        button->setStyleClass("btn btn-danger");
        button->setToolTip("Break up partnership");
        button->clicked().connect(std::bind([=] () {
            auto unique_dialog = std::make_unique<Confirmation_dialog>(
                    "Are you sure you want to break up this partnership? "
                    "Their submission will be cleared, and they will have "
                    "to resubmit."
            );
            auto dialog = impl_->addChild(std::move(unique_dialog));
            dialog->accepted().connect(
                    this, &Submission_owner_widget::break_up_partnership_);
        }));
    }

    else if (!submission_->user2() && submission_->assignment()->partner()) {
        impl_->addNew<Wt::WBreak>();
        impl_->addNew<Wt::WText>("Partner with: ");
        auto edit = impl_->addNew<Wt::WLineEdit>();
        edit->setStyleClass("username");
        edit->setPlaceholderText("NetID");
        edit->enterPressed().connect(std::bind([=] () {
            dbo::Transaction transaction2(session_);

            auto user2 = User::find_by_name(session_, edit->text().toUTF8());
            if (!user2) {
                edit->setText("");
                return;
            };

            auto other = Submission::find_by_assignment_and_user(
                    session_, submission_->assignment(), user2);
            if (!other) return;

            Submission::join_together(submission_, other);
            Partner_request::delete_requests(session_, submission_->user1(),
                                             submission_->assignment());
            Partner_request::delete_requests(session_, submission_->user2(),
                                             submission_->assignment());

            transaction2.commit();

            changed_.emit();
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

    impl_->addNew<Wt::WText>(message.str());
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
        impl_->addNew<Wt::WText>(message.str());
        return;
    }

    impl_->addNew<Partner_notification_widget>(self, submission_, session_);
}

void Submission_owner_widget::break_up_partnership_()
{
    dbo::Transaction transaction(session_);
    auto mutable_submission = submission_.modify();
    mutable_submission->set_user2({});
    mutable_submission->clear_files();
    transaction.commit();

    changed_.emit();
    update_();
}

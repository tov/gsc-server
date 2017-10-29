#include "Admin_view.h"
#include "Accelerator_button.h"
#include "Accelerator_text.h"
#include "File_manager_view.h"
#include "Held_back_view.h"
#include "User_suggester.h"
#include "game/HangmanWidget.h"
#include "game/HighScoresWidget.h"
#include "../Navigate.h"
#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../model/Submission.h"
#include "../model/Session.h"

#include <Wt/WApplication>
#include <Wt/WComboBox>
#include <Wt/WLineEdit>
#include <Wt/WMenuItem>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WSuggestionPopup>
#include <Wt/WTable>

#include <sstream>
#include <vector>

class Submission_chooser : public Wt::WContainerWidget
{
public:
    Submission_chooser(Session& session,
                       Wt::WContainerWidget* parent = nullptr);

    using This = Submission_chooser;

    virtual void setFocus(bool) override;

private:
    void update();
    void go();

    Session& session_;
    Wt::WLineEdit* editor_;
    Wt::WComboBox* combo_;
    std::vector<Wt::Dbo::ptr<Submission>> submissions_;
};

Submission_chooser::Submission_chooser(Session& session,
                                       Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          session_(session)
{
    editor_ = new Wt::WLineEdit(this);
    editor_->setEmptyText("username");
    editor_->keyWentUp().connect(this, &This::update);
    editor_->enterPressed().connect(this, &This::go);

    auto popup = new User_suggester(session, User::Role::Student, this);
    popup->forEdit(editor_);

    combo_  = new Wt::WComboBox(this);
    combo_->changed().connect(this, &This::go);
    combo_->enterPressed().connect(this, &This::go);
}

void Submission_chooser::update()
{
    combo_->clear();
    submissions_.clear();

    Wt::Dbo::Transaction transaction(session_);

    auto user = User::find_by_name(session_, editor_->text().toUTF8());

    if (user) {
        submissions_ = user->submissions();

        for (const auto& submission : submissions_) {
            auto number = submission->assignment()->number();
            combo_->addItem(boost::lexical_cast<std::string>(number));
        }
    }
}

void Submission_chooser::go()
{
    auto current_index = combo_->currentIndex();

    if (current_index < submissions_.size()) {
        Wt::Dbo::Transaction transaction(session_);

        auto submission = submissions_[size_t(current_index)];

        switch (submission->status()) {
            case Submission::Status::future:
            case Submission::Status::open:
            case Submission::Status::extended:
                Navigate::to(submission->url());

            case Submission::Status::self_eval:
            case Submission::Status::extended_eval:
            case Submission::Status::closed:
                Navigate::to(submission->eval_url());
                break;
        }
    }
}

void Submission_chooser::setFocus(bool b)
{
    editor_->setFocus(b);
}

class Student_chooser : public Wt::WContainerWidget
{
public:
    Student_chooser(Session& session,
                    Wt::WContainerWidget* parent = nullptr);

    virtual void setFocus(bool) override;

private:
    void go();

    Session& session_;
    Wt::WLineEdit* editor_;
};

Student_chooser::Student_chooser(Session& session, Wt::WContainerWidget* parent)
        : WContainerWidget(parent), session_(session)
{
    editor_ = new Wt::WLineEdit(this);
    editor_->setEmptyText("username");
    editor_->enterPressed().connect(this, &Student_chooser::go);

    auto popup = new User_suggester(session, User::Role::Student, this);
    popup->forEdit(editor_);
}

void Student_chooser::go()
{
    Wt::Dbo::Transaction transaction(session_);
    dbo::ptr<User> user = User::find_by_name(session_, editor_->text().toUTF8());
    transaction.commit();

    if (!user) return;

    Navigate::to("/~" + user->name() + "/hw");
}

void Student_chooser::setFocus(bool b)
{
    editor_->setFocus(b);
}

class Role_chooser : public Wt::WContainerWidget
{
public:
    Role_chooser(Session& session,
                 Wt::WContainerWidget* parent = nullptr);

    using This = Role_chooser;

    virtual void setFocus(bool) override;

private:
    void update();
    void go();

    Session& session_;
    Wt::WLineEdit* editor_;
    Wt::WComboBox* combo_;
};

Role_chooser::Role_chooser(Session& session,
                           Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          session_(session)
{
    editor_ = new Wt::WLineEdit(this);
    editor_->setEmptyText("username");
    editor_->keyWentUp().connect(this, &This::update);

    auto popup = new User_suggester(session, this);
    popup->forEdit(editor_);

    combo_  = new Wt::WComboBox(this);
    combo_->enterPressed().connect(this, &This::go);
    combo_->changed().connect(this, &This::go);
}

void Role_chooser::update()
{
    combo_->clear();

    Wt::Dbo::Transaction transaction(session_);
    auto user = User::find_by_name(session_, editor_->text().toUTF8());
    transaction.commit();

    if (user) {
        // This order of items must match the order in the User::Role enum:
        combo_->addItem("Student");
        combo_->addItem("Grader");
        combo_->addItem("Admin");
        combo_->setCurrentIndex((int) user->role());
    }
}

void Role_chooser::go()
{
    // magic number must match number of User::Role enum values:
    if (combo_->currentIndex() < 3) {
        Wt::Dbo::Transaction transaction(session_);
        auto user = User::find_by_name(session_, editor_->text().toUTF8());
        user.modify()->set_role((User::Role) combo_->currentIndex());
        transaction.commit();
    }
}

void Role_chooser::setFocus(bool b)
{
    editor_->setFocus(b);
}

class SU_widget : public Wt::WContainerWidget
{
public:
    SU_widget(Session& session,
              Wt::WContainerWidget* parent = nullptr);

    using This = SU_widget;

    virtual void setFocus(bool) override;

private:
    Session& session_;
    Wt::WLineEdit* editor_;

    void go();
};

SU_widget::SU_widget(Session& session, Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          session_(session)
{
    editor_ = new Wt::WLineEdit(this);
    editor_->setEmptyText("username");
    editor_->enterPressed().connect(this, &This::go);

    auto popup = new User_suggester(session, this);
    popup->forEdit(editor_);
}

void SU_widget::go()
{
    dbo::Transaction transaction(session_);

    auto user = User::find_by_name(session_, editor_->text().toUTF8());
    if (!user) return;

    session_.become_user(user);
}

void SU_widget::setFocus(bool b)
{
    editor_->setFocus(b);
}

Admin_view::Admin_view(Session& session, Wt::WContainerWidget* parent)
        : WContainerWidget(parent), session_(session)
{
    setStyleClass("admin-view");

    auto table = new Wt::WTable(this);
    int row = 0;

    auto ja = new Accelerator_text("Jump to su&bmission",
                                   table->elementAt(row, 0));
    ja->set_target(new Submission_chooser(session_, table->elementAt(row, 1)));

    auto js = new Accelerator_text("Jump to &student:",
                                   table->elementAt(++row, 0));
    js->set_target(new Student_chooser(session_, table->elementAt(row, 1)));

    auto cr = new Accelerator_text("Change &role:", table->elementAt(++row, 0));
    cr->set_target(new Role_chooser(session_, table->elementAt(row, 1)));

    auto su = new Accelerator_text("S&witch users:",
                                   table->elementAt(++row, 0));
    su->set_target(new SU_widget(session_, table->elementAt(row, 1)));

    auto gr = new Accelerator_button("&Grade", table->elementAt(++row, 1));
    gr->clicked().connect(Navigate("/grade"));

    auto hw = new Accelerator_button("Edit &assignments",
                                     table->elementAt(++row, 1));
    hw->clicked().connect(Navigate("/hw"));

    auto gs = new Accelerator_button("Grading s&tats", table->elementAt
                                                                    (++row, 1));
    gs->clicked().connect(Navigate("/grading_stats"));

    auto play_game = new Accelerator_button("&Play game",
                                            table->elementAt(++row, 1));
    play_game->clicked().connect(Navigate("/game"));

    new Held_back_view(session_, this);
}

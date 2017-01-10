#include "Admin_view.h"
#include "Accelerator_button.h"
#include "Accelerator_text.h"
#include "File_manager_view.h"
#include "User_suggester.h"
#include "game/HangmanWidget.h"
#include "game/HighScoresWidget.h"
#include "../Navigate.h"
#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../model/Submission.h"

#include <Wt/WApplication>
#include <Wt/WComboBox>
#include <Wt/WLineEdit>
#include <Wt/WMenuItem>
#include <Wt/WMessageBox>
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
    editor_->changed().connect(this, &This::update);
    editor_->keyWentDown().connect(this, &This::update);
    editor_->enterPressed().connect(this, &This::go);

    auto popup = new User_suggester(session, User::Role::Student, this);
    popup->forEdit(editor_);

    combo_  = new Wt::WComboBox(this);
    combo_->enterPressed().connect(this, &This::go);
    combo_->changed().connect(this, &This::go);
}

void Submission_chooser::update()
{
    combo_->clear();
    submissions_.clear();

    Wt::Dbo::Transaction transaction(session_);

    auto user = User::find_by_name(session_, editor_->text().toUTF8());

    if (user) {
        bool focus = false;

        submissions_ = user->submissions();

        for (const auto& submission : submissions_) {
            auto number = submission->assignment()->number();
            combo_->addItem(boost::lexical_cast<std::string>(number));
            focus = true;
        }

        if (focus) combo_->setFocus();
    }
}

void Submission_chooser::go()
{
    if (combo_->currentIndex() < submissions_.size()) {
        Wt::Dbo::Transaction transaction(session_);

        auto submission = submissions_[size_t(combo_->currentIndex())];

        std::ostringstream path;
        path << "/~" << submission->user1()->name();
        path << "/hw/" << submission->assignment()->number();

        Navigate::to(path.str());
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
    editor_->changed().connect(this, &This::update);
    editor_->keyWentDown().connect(this, &This::update);
    editor_->enterPressed().connect(this, &This::go);

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

Admin_view::Admin_view(Session& session, Wt::WContainerWidget* parent)
        : WContainerWidget(parent), session_(session)
{
    setStyleClass("admin-view");

    auto table = new Wt::WTable(this);

    auto ja = new Accelerator_text("Jump to su&bmission",
                                   table->elementAt(0, 0));
    ja->set_target(new Submission_chooser(session_, table->elementAt(0, 1)));

    auto js = new Accelerator_text("Jump to &student:", table->elementAt(1, 0));
    js->set_target(new Student_chooser(session_, table->elementAt(1, 1)));

    auto cr = new Accelerator_text("Change &role:", table->elementAt(2, 0));
    cr->set_target(new Role_chooser(session_, table->elementAt(2, 1)));

    auto hw = new Accelerator_button("Edit &assignments",
                                     table->elementAt(3, 1));
    hw->clicked().connect(Navigate("/hw"));

    auto play_game = new Accelerator_button("&Play game",
                                            table->elementAt(4, 1));
    play_game->clicked().connect(Navigate("/game"));
}

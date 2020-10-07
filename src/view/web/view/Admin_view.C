#include "../../../Navigate.h"
#include "../../../Session.h"
#include "../../../common/util.h"
#include "../../../model/Assignment.h"
#include "../../../model/Submission.h"
#include "../../game/HangmanWidget.h"
#include "../widget/Accelerator_button.h"
#include "../widget/Accelerator_text.h"
#include "../widget/User_selector.h"
#include "../widget/User_suggester.h"
#include "Admin_view.h"
#include "Held_back_view.h"

#include <Wt/WComboBox.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WEvent.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMenuItem.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WValidator.h>

#include <vector>

class Submission_chooser : public WContainerWidget {
public:
  Submission_chooser(Session &session);

  using This = Submission_chooser;

  virtual void setFocus(bool) override;

private:
  void update();
  void go();

  Session &session_;
  User_selector *editor_;
  WComboBox *combo_;
  vector<dbo::ptr<Submission>> submissions_;
};

Submission_chooser::Submission_chooser(Session &session) : session_(session) {
  editor_ = addNew<User_selector>(session_, User::Role::Student);
  editor_->textInput().connect(this, &This::update);
  editor_->enterPressed().connect(this, &This::go);

  combo_ = addNew<WComboBox>();
  combo_->changed().connect(this, &This::go);
  combo_->enterPressed().connect(this, &This::go);
}

void Submission_chooser::update() {
  combo_->clear();
  submissions_.clear();

  dbo::Transaction transaction(session_);

  auto user = User::find_by_name(session_, editor_->text().toUTF8());

  if (user) {
    submissions_ = user->submissions();

    for (const auto &submission : submissions_) {
      auto number = submission->assignment()->number();
      combo_->addItem(to_string(number));
    }
  }
}

void Submission_chooser::go() {
  int current_index = combo_->currentIndex();

  if (current_index < (int)submissions_.size()) {
    dbo::Transaction transaction(session_);

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

void Submission_chooser::setFocus(bool b) { editor_->setFocus(b); }

class Student_chooser : public WContainerWidget {
public:
  Student_chooser(Session &session);

  virtual void setFocus(bool) override;

private:
  void go();

  Session &session_;
  User_selector *editor_;
};

Student_chooser::Student_chooser(Session &session) : session_(session) {
  editor_ = addNew<User_selector>(session_, User::Role::Student);
  editor_->enterPressed().connect(this, &Student_chooser::go);
}

void Student_chooser::go() {
  if (auto user = editor_->selected_user())
    Navigate::to(user->hw_url());
}

void Student_chooser::setFocus(bool b) { editor_->setFocus(b); }

class Role_chooser : public WCompositeWidget {
public:
  explicit Role_chooser(Session &, optional<User::Role> = nullopt);

  using Role = User::Role;
  using This = Role_chooser;

  optional<Role> role();
  void setRole(optional<Role>);

  EventSignal<> &changed() { return combo_->changed(); }
  EventSignal<> &enterPressed() { return combo_->enterPressed(); }

private:
  [[maybe_unused]] Session &session_;
  WComboBox *combo_;
};

class Role_updater : public WContainerWidget {
public:
  explicit Role_updater(Session &session);

  using This = Role_updater;

  virtual void setFocus(bool) override;

private:
  void update();
  void go();

  Session &session_;
  User_selector *editor_;
  Role_chooser *chooser_;
};

Role_chooser::Role_chooser(Session &session, optional<Role> role)
    : WCompositeWidget(make_unique<WComboBox>()), session_(session),
      combo_(dynamic_cast<WComboBox *>(implementation())) {
  combo_->setNoSelectionEnabled(true);

  for (auto const &each : Enum<Role>::info)
    combo_->addItem(each.name);

  setRole(role);
}

optional<User::Role> Role_chooser::role() {
  return Enum<Role>::from_repr(combo_->currentIndex());
}

void Role_chooser::setRole(optional<Role> role) {
  if (role) {
    combo_->setDisabled(false);
    combo_->setCurrentIndex(Enum<Role>::to_repr(*role));
  } else {
    combo_->setCurrentIndex(-1);
    combo_->setDisabled(true);
  }
}

Role_updater::Role_updater(Session &session) : session_(session) {
  editor_ = addNew<User_selector>(session_);
  editor_->textInput().connect(this, &This::update);

  chooser_ = addNew<Role_chooser>(session);
  chooser_->enterPressed().connect(this, &This::go);
}

void Role_updater::update() {
  if (auto user = editor_->selected_user())
    chooser_->setRole(user->role());
  else
    chooser_->setRole(nullopt);
}

void Role_updater::go() {
  if (auto choice = chooser_->role()) {
    if (auto user = editor_->selected_user()) {
      dbo::Transaction transaction(session_);
      user.modify()->set_role(*choice);
    }

    chooser_->setRole(nullopt);
    editor_->setText("");
    editor_->setFocus(true);
  }
}

void Role_updater::setFocus(bool b) { editor_->setFocus(b); }

class SU_widget : public WContainerWidget {
public:
  SU_widget(Session &session);

  using This = SU_widget;

  virtual void setFocus(bool) override;

private:
  Session &session_;
  User_selector *editor_;

  void go();
};

SU_widget::SU_widget(Session &session) : session_(session) {
  editor_ = addNew<User_selector>(session_);
  editor_->enterPressed().connect(this, &This::go);
}

void SU_widget::go() {
  if (auto user = editor_->selected_user())
    session_.become_user(user);
}

void SU_widget::setFocus(bool b) { editor_->setFocus(b); }

class New_user_widget : public WContainerWidget {
public:
  New_user_widget(Session &session);

  using This = New_user_widget;

  virtual void setFocus(bool) override;

private:
  Session &session_;
  WLineEdit *editor_;

  void go();
};

New_user_widget::New_user_widget(Session &session) : session_(session) {
  editor_ = addNew<WLineEdit>();
  editor_->setPlaceholderText("username");
  editor_->setStyleClass("username");
  editor_->enterPressed().connect(this, &This::go);
}

void New_user_widget::go() {
  dbo::Transaction transaction(session_);

  if (string username = clean_username(editor_->text()); !username.empty())
    session_.create_user({username});

  editor_->setText("");
}

void New_user_widget::setFocus(bool b) { editor_->setFocus(b); }

Admin_view::Admin_view(Session &session) : session_(session) {
  setStyleClass("admin-view");

  auto table = addNew<WTable>();
  int row = 0;

  table->setHeaderCount(1, Orientation::Vertical);

  auto ja =
      table->elementAt(row, 0)->addNew<Accelerator_text>("Jump to su&bmission");
  ja->set_target(
      table->elementAt(row, 1)->addNew<Submission_chooser>(session_));

  auto js =
      table->elementAt(++row, 0)->addNew<Accelerator_text>("Jump to &student:");
  js->set_target(table->elementAt(row, 1)->addNew<Student_chooser>(session_));

  auto cr =
      table->elementAt(++row, 0)->addNew<Accelerator_text>("Change &role:");
  cr->set_target(table->elementAt(row, 1)->addNew<Role_updater>(session_));

  auto su =
      table->elementAt(++row, 0)->addNew<Accelerator_text>("S&witch users:");
  su->set_target(table->elementAt(row, 1)->addNew<SU_widget>(session_));

  auto nu = table->elementAt(++row, 0)->addNew<Accelerator_text>("&New user:");
  nu->set_target(table->elementAt(row, 1)->addNew<New_user_widget>(session_));

  auto gr = table->elementAt(++row, 1)->addNew<Accelerator_button>("&Grade");
  gr->clicked().connect(Navigate("/grade"));

  auto hw = table->elementAt(++row, 1)->addNew<Accelerator_button>(
      "Edit &assignments");
  hw->clicked().connect(Navigate("/hw"));

  auto gs =
      table->elementAt(++row, 1)->addNew<Accelerator_button>("Grading s&tats");
  gs->clicked().connect(Navigate("/grading_stats"));

  auto play_game =
      table->elementAt(++row, 1)->addNew<Accelerator_button>("&Play game");
  play_game->clicked().connect(Navigate("/game"));

  addNew<Held_back_view>(session_);
}

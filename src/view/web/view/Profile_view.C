#include "../../../Session.h"
#include "../../../common/util.h"
#include "../../../model/auth/User.h"
#include "../widget/Api_key_widget.h"
#include "../widget/Glyph_button.h"
#include "Profile_view.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

namespace templates {

WString const profile_table = WString::tr("gsc.template.profile-table"),
              view_widget = WString::tr("gsc.template.simple-view-widget"),
              edit_widget = WString::tr("gsc.template.simple-edit-widget");

}

class Profile_view::Call_me_widget : public WCompositeWidget {
public:
  Call_me_widget(Profile_view &parent);

private:
  Profile_view &parent_;
  WContainerWidget *impl_;
  WLineEdit *line_edit_;
  Glyph_button *save_btn_ = nullptr;

  void view_mode_();
  void edit_mode_();
  void save_();

  void on_edit_();

  bool is_edited_() const;
  std::string const &value_() const;
};

Profile_view::Profile_view(dbo::ptr<User> const &user, Session &session)
    : user_(user), session_(session) {
  impl_ = setNewImplementation<WTemplate>(templates::profile_table);

  impl_->setStyleClass("profile-view");
  impl_->bindString("login-name", user->name());
  impl_->bindNew<Call_me_widget>("call-me", *this);
  impl_->bindString("role", user->role_string());
  impl_->bindNew<Api_key_widget>("api-key", user_, session_);
}

using Call_me = Profile_view::Call_me_widget;

Call_me::Call_me_widget(Profile_view &parent) : parent_(parent) {
  impl_ = setNewImplementation<WContainerWidget>();
  view_mode_();
}

void Call_me::view_mode_() {
  impl_->clear();
  auto root = impl_->addNew<WTemplate>(templates::view_widget);
  line_edit_ = root->bindNew<WLineEdit>("value", value_());
  auto edit_btn = root->bindNew<Glyph_button>("edit-button", "edit", "Edit");
  save_btn_ = nullptr;

  line_edit_->clicked().connect(this, &Call_me::edit_mode_);
  line_edit_->focussed().connect(this, &Call_me::edit_mode_);
  edit_btn->clicked().connect(this, &Call_me::edit_mode_);
  edit_btn->setStyleClass("btn btn-success");
}

void Call_me::edit_mode_() {
  impl_->clear();
  auto root = impl_->addNew<WTemplate>(templates::edit_widget);

  line_edit_ = root->bindNew<WLineEdit>("edit", value_());
  save_btn_ = root->bindNew<Glyph_button>("save-button", "save", "Save");
  auto cancel_btn =
      root->bindNew<Glyph_button>("cancel-button", "cancel", "Cancel");

  line_edit_->keyWentUp().connect(this, &Call_me::on_edit_);
  cancel_btn->clicked().connect(this, &Call_me::view_mode_);
  save_btn_->clicked().connect(this, &Call_me::save_);

  line_edit_->setMaxLength(MAX_CALL_ME_LENGTH);
  cancel_btn->setStyleClass("btn");
  save_btn_->setStyleClass("btn btn-primary");

  save_btn_->disable();
  line_edit_->setFocus();
}

void Call_me::save_() {
  dbo::Transaction trans(parent_.session_);

  if (is_edited_())
    parent_.user_.modify()->set_call_me(line_edit_->text().toUTF8());

  trans.commit();

  view_mode_();
}

void Profile_view::Call_me_widget::on_edit_() {
  if (save_btn_)
    save_btn_->setEnabled(is_edited_());
}

bool Profile_view::Call_me_widget::is_edited_() const {
  return value_() != line_edit_->text();
}

std::string const &Call_me::value_() const { return parent_.user_->call_me(); }

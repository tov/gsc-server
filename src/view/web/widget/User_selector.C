#include "../../../Session.h"
#include "../../../common/util.h"
#include "../Username_validator.h"
#include "User_selector.h"
#include "User_suggester.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WRegExpValidator.h>

#include <cctype>
#include <sstream>

User_selector::User_selector(Session &session, optional<User::Role> role)
    : session_(session), valid_(make_shared<Username_validator>()) {
  auto impl = setNewImplementation<WContainerWidget>();

  setStyleClass("user-selector");

  edit_ = impl->addNew<WLineEdit>();
  edit_->setPlaceholderText("username");
  edit_->setStyleClass("username");

  edit_->setValidator(valid_);

  auto popup = make_unique<User_suggester>(session, role);
  popup->forEdit(edit_);
  impl->addChild(move(popup));

  textInput().connect(this, &User_selector::fixup_);
}

string User_selector::cleaned_username() const {
  return clean_username(text());
}

dbo::ptr<User> User_selector::selected_user() const {
  Wt::Dbo::Transaction transaction(session_);
  return User::find_by_name(session_, cleaned_username());
}

void User_selector::fixup_() { setText(clean_username(text())); }

string clean_username(Wt::WString const &text) {
  ostringstream os;

  for (char c : text.toUTF8()) {
    if (isalnum(c)) {
      os << char(tolower(c));
    }
  }

  return os.str();
}

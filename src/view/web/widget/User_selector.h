#pragma once

#include "../../../model/auth/User.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WEvent.h>
#include <Wt/WLineEdit.h>

#include <memory>
#include <optional>

class Session;

#pragma push_macro("DELEGATE")

#define DELEGATE(dest, name, cv)                                               \
  template <class... Arg> decltype(auto) name(Arg &&... arg) cv {              \
    return dest->name(std::forward<Arg>(arg)...);                              \
  }

std::string clean_username(Wt::WString const &text);

class User_selector : public Wt::WCompositeWidget {
public:
  explicit User_selector(Session &, std::optional<User::Role> = std::nullopt);

  std::string cleaned_username() const;

  Wt::Dbo::ptr<User> selected_user() const;

  DELEGATE(edit_, enterPressed, )
  DELEGATE(edit_, changed, )
  DELEGATE(edit_, keyWentDown, )
  DELEGATE(edit_, keyWentUp, )
  DELEGATE(edit_, textInput, )
  DELEGATE(edit_, text, const)
  DELEGATE(edit_, setText, )
  DELEGATE(edit_, setFocus, )

private:
  Session &session_;
  std::shared_ptr<Wt::WValidator> valid_;
  Wt::WLineEdit *edit_;

  void fixup_();
};

#pragma pop_macro("DELEGATE")

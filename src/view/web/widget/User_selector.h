#pragma once

#include "../../../model/auth/User.h"
#include <Wt/WCompositeWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WEvent.h>
#include <Wt/Dbo/ptr.h>
#include <optional>

class Session;

#pragma push_macro("DELEGATE")
#pragma push_macro("DELEGATE1")

#define DELEGATE(dest,name) \
    decltype(auto) name() const { return dest->name(); }
#define DELEGATE1(dest,name,T1) \
    decltype(auto) name(T1 A1) { return dest->name(A1); }

std::string clean_text(Wt::WString const&text);

class User_selector : public Wt::WCompositeWidget
{
private:
    Session& session_;
    Wt::WLineEdit* edit_;

public:
    explicit User_selector(Session&,
                           std::optional<User::Role> = std::nullopt);

    std::string cleaned_text() const;

    Wt::Dbo::ptr<User> selected_user() const;

    DELEGATE(edit_, enterPressed)
    DELEGATE(edit_, keyWentUp)
    DELEGATE(edit_, keyWentDown)
    DELEGATE(edit_, text)
    DELEGATE1(edit_, setText, Wt::WString const&)
};

#pragma pop_macro("DELEGATE")
#pragma pop_macro("DELEGATE1")

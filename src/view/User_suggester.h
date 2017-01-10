#pragma once

#include "../model/auth/User.h"

#include <Wt/WSuggestionPopup>
#include <Wt/WContainerWidget>

class Session;

class User_suggester : public Wt::WSuggestionPopup
{
public:
    User_suggester(Session&,
                   User::Role role,
                   Wt::WContainerWidget* parent = nullptr);
    User_suggester(Session&,
                   Wt::WContainerWidget* parent = nullptr);

private:
    static const Wt::WSuggestionPopup::Options options;
};


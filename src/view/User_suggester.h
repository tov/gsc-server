#pragma once

#include "../model/auth/User.h"

#include <Wt/WSuggestionPopup.h>
#include <Wt/WContainerWidget.h>

class Session;

class User_suggester : public Wt::WSuggestionPopup
{
public:
    User_suggester(Session&, User::Role role);
    User_suggester(Session&);

private:
    static const Wt::WSuggestionPopup::Options options;
};


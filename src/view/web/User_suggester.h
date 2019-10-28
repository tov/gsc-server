#pragma once

#include "../../model/auth/User.h"

#include <Wt/WSuggestionPopup.h>

class Session;

class User_suggester : public Wt::WSuggestionPopup
{
public:
    User_suggester(Session&, User::Role role);
    explicit User_suggester(Session&);
};


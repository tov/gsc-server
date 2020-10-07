#pragma once

#include "../../../model/auth/User.h"
#include <Wt/WSuggestionPopup.h>
#include <optional>

class Session;

class User_suggester : public Wt::WSuggestionPopup {
public:
  explicit User_suggester(Session &, std::optional<User::Role> = std::nullopt);
};

#pragma once

#include "../specializations.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WDateTime.h>

namespace dbo = Wt::Dbo;

class User;

class User_stats {
public:
  User_stats(){};
  User_stats(const dbo::ptr<User> &);

  dbo::ptr<User> user() const { return user_; }
  int games_played() const { return games_played_; }
  long long score() const { return score_; }
  const Wt::WDateTime &last_game() const { return last_game_; }

  void record_game(int score);

private:
  dbo::ptr<User> user_;
  int games_played_ = 0;
  long long score_ = 0;
  Wt::WDateTime last_game_;

public:
  template <class Action> void persist(Action &a) {
    dbo::id(a, user_, "user", dbo::OnDeleteCascade);
    dbo::field(a, games_played_, "games_played");
    dbo::field(a, score_, "score");
    dbo::field(a, last_game_, "last_game");
  }
};

DBO_EXTERN_TEMPLATES(User_stats)

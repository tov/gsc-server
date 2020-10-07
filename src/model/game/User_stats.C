#include "../auth/User.h"
#include "User_stats.h"

DBO_INSTANTIATE_TEMPLATES(User_stats)

User_stats::User_stats(const dbo::ptr<User> &user) : user_(user) {}

void User_stats::record_game(int score) {
  score_ += score;
  ++games_played_;
  last_game_ = Wt::WDateTime::currentDateTime();
}

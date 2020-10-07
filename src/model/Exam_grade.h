#pragma once

#include <Wt/Dbo/Dbo.h>
#include <Wt/Json/Object.h>
#include <string>

namespace dbo = Wt::Dbo;

class User;

class Exam_grade {
public:
  Exam_grade() {}
  Exam_grade(const dbo::ptr<User> &user, int number);

  const dbo::ptr<User> &user() const { return user_; }
  int number() const { return number_; }
  int points() const { return points_; }
  int possible() const { return possible_; }

  void set_points_and_possible(int points, int possible);

  double grade() const;
  std::string pct_string() const;

  Wt::Json::Object to_json() const;

  static dbo::collection<dbo::ptr<Exam_grade>>
  find_by_user(const dbo::ptr<User> &);

  static dbo::ptr<Exam_grade> find_by_user_and_number(const dbo::ptr<User> &,
                                                      int number);

private:
  dbo::ptr<User> user_;
  int number_;
  int points_;
  int possible_;

public:
  template <typename Action> void persist(Action &a) {
    dbo::belongsTo(a, user_, "user", dbo::OnDeleteCascade);
    dbo::field(a, number_, "number");
    dbo::field(a, points_, "points");
    dbo::field(a, possible_, "possible");
  }
};

DBO_EXTERN_TEMPLATES(Exam_grade)

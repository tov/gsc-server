#pragma once

#include "specializations.h"

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WDateTime.h>

#include <string>
#include <vector>

namespace dbo = Wt::Dbo;

class Assignment;
class Eval_item;
class Submission;

using Eval_items = dbo::collection<dbo::ptr<Eval_item>>;
using Eval_item_vec = std::vector<dbo::ptr<Eval_item>>;
using Submissions = dbo::collection<dbo::ptr<Submission>>;

class Assignment {
public:
  enum class Status {
    Future,
    Open,
    Self_eval,
    Closed,
  };

  Assignment() {}

  Assignment(int number, const std::string &name, int points,
             const Wt::WDateTime &open_date, const Wt::WDateTime &due_date,
             const Wt::WDateTime &eval_date);

  int number() const { return number_; }
  const std::string &name() const { return name_; }
  int points() const { return points_; }
  const Wt::WDateTime &open_date() const { return open_date_; }
  const Wt::WDateTime &due_date() const { return due_date_; }
  const Wt::WDateTime &eval_date() const { return eval_date_; }
  bool partner() const { return partner_; }
  bool web_allowed() const { return web_allowed_; }
  Eval_items eval_items() const;
  Eval_item_vec eval_item_vec() const;
  Submissions submissions() const { return submissions_; }

  void set_name(const std::string &name) { name_ = name; }
  void set_points(int points) { points_ = points; }
  void set_open_date(const Wt::WDateTime &date) { open_date_ = date; }
  void set_due_date(const Wt::WDateTime &date) { due_date_ = date; }
  void set_eval_date(const Wt::WDateTime &date) { eval_date_ = date; }
  void set_partner(bool b) { partner_ = b; }
  void set_web_allowed(bool b) { web_allowed_ = b; }

  std::string slug_string() const;

  Status status() const;

  double total_relative_value() const;

  static dbo::ptr<Assignment> find_by_number(dbo::Session &, int);

  dbo::ptr<Eval_item> find_eval_item(dbo::Session &, int) const;

private:
  int number_ = 0;
  std::string name_;
  int points_ = 0;
  bool partner_ = false;
  bool web_allowed_ = false;
  Wt::WDateTime open_date_;
  Wt::WDateTime due_date_;
  Wt::WDateTime eval_date_;
  Eval_items eval_items_;
  Submissions submissions_;

public:
  template <typename Action> void persist(Action &a) {
    dbo::id(a, number_, "number");
    dbo::field(a, name_, "name");
    dbo::field(a, points_, "points");
    dbo::field(a, partner_, "partner");
    dbo::field(a, web_allowed_, "web_allowed");
    dbo::field(a, open_date_, "open_date");
    dbo::field(a, due_date_, "due_date");
    dbo::field(a, eval_date_, "eval_date");
    dbo::hasMany(a, eval_items_, dbo::ManyToOne, "assignment");
    dbo::hasMany(a, submissions_, dbo::ManyToOne, "assignment");
  }
};

DBO_EXTERN_TEMPLATES(Assignment)

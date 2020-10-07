#pragma once

#include "../common/stringify.h"
#include "specializations.h"

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Json/Object.h>

#include <iostream>
#include <optional>
#include <string>

namespace dbo = Wt::Dbo;

class Assignment;
class User;
class Self_eval;
class Submission;

using Self_evals = dbo::collection<dbo::ptr<Self_eval>>;

class Eval_item {
public:
  enum class Type : int {
    Boolean,
    Scale,
    Informational,
  };

  Eval_item() {}
  Eval_item(const dbo::ptr<Assignment> &, int sequence);

  const dbo::ptr<Assignment> &assignment() const { return assignment_; }
  int sequence() const { return sequence_; }
  Type type() const { return static_cast<Type>(type_); }
  const std::string &prompt() const { return prompt_; }
  double relative_value() const { return relative_value_; }
  Self_evals self_evals() const { return self_evals_; }

  void set_type(Type t) { type_ = static_cast<int>(t); }
  void set_prompt(const std::string &p) { prompt_ = p; }
  void set_relative_value(double d) { relative_value_ = d; }

  std::string relative_value_str(int precision = 3) const;
  void set_relative_value(const std::string &);

  double absolute_value() const;
  std::string absolute_value_str(int precision = 3) const;

  std::string format_score(double, bool for_grader = false) const;

  bool is_informational() const;
  bool is_graded_automatically() const;

  std::string rest_uri(dbo::ptr<Submission> const &as_part_of) const;
  Wt::Json::Object to_json(Wt::Dbo::ptr<Submission> const &as_part_of,
                           dbo::ptr<User> const &as_seen_by,
                           bool brief = false) const;

private:
  dbo::ptr<Assignment> assignment_;
  int sequence_ = 0;
  int type_ = 0;
  std::string prompt_;
  double relative_value_ = 1.0;
  Self_evals self_evals_;

public:
  template <class Action> void persist(Action &a) {
    dbo::belongsTo(a, assignment_, "assignment", dbo::OnDeleteCascade);
    dbo::field(a, sequence_, "sequence");
    dbo::field(a, type_, "type");
    dbo::field(a, prompt_, "prompt");
    dbo::field(a, relative_value_, "relative_value");
    dbo::hasMany(a, self_evals_, dbo::ManyToOne, "eval_item");
  }
};

template <> struct Enum<Eval_item::Type> {
  static char const *show(Eval_item::Type);
  static Eval_item::Type read(const char *);
};

std::ostream &operator<<(std::ostream &, Eval_item::Type);

DBO_EXTERN_TEMPLATES(Eval_item)

#include "../common/format.h"
#include "../common/paths.h"
#include "Assignment.h"
#include "Eval_item.h"
#include "Grader_eval.h"
#include "Self_eval.h"
#include "Submission.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Json/Value.h>

#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace J = Wt::Json;

DBO_INSTANTIATE_TEMPLATES(Eval_item)

Eval_item::Eval_item(const dbo::ptr<Assignment> &assignment, int sequence)
    : assignment_(assignment), sequence_(sequence) {}

std::string Eval_item::relative_value_str(int precision) const {
  std::ostringstream os;
  os << std::setprecision(precision) << relative_value_;
  return os.str();
}

void Eval_item::set_relative_value(const std::string &s) {
  set_relative_value(atof(s.data()));
}

std::string Eval_item::format_score(double score, bool for_grader) const {
  switch (type()) {
  case Type::Boolean:
    if (score == 0)
      return "No";
    if (score == 1)
      return "Yes";
    else
      return percentage(score);

  default:
    if (for_grader)
      return "✓";
    else
      return percentage(score);
  }
}

bool Eval_item::is_informational() const {
  return type() == Type::Informational;
}

double Eval_item::absolute_value() const {
  double denominator = assignment()->total_relative_value();
  return denominator == 0 ? 0 : relative_value() / denominator;
}

std::string Eval_item::absolute_value_str(int precision) const {
  return percentage(absolute_value(), precision);
}

std::string Eval_item::rest_uri(dbo::ptr<Submission> const &as_part_of) const {
  return api::paths::Submissions_1_evals_2(as_part_of.id(), sequence());
}

J::Object Eval_item::to_json(dbo::ptr<Submission> const &as_part_of,
                             dbo::ptr<User> const &as_seen_by,
                             bool brief) const {
  J::Object result;

  result["uri"] = J::Value(rest_uri(as_part_of));
  result["sequence"] = J::Value(sequence());
  result["submission_uri"] = J::Value(as_part_of->rest_uri());
  result["type"] = J::Value(stringify(type()));

  if (!brief) {
    result["prompt"] = J::Value(prompt());
    result["value"] = J::Value(relative_value());

    // Get the self eval, creating it if the type is informational.
    auto self_eval = Submission::get_self_eval(sequence(), as_part_of,
                                               type() == Type::Informational);
    if (self_eval) {
      result["self_eval"] = self_eval->to_json({as_seen_by});

      auto grader_eval = self_eval->grader_eval();
      if (grader_eval && grader_eval->can_view(as_seen_by)) {
        result["grader_eval"] = grader_eval->to_json({as_seen_by});
      }
    }
  }

  return result;
}

bool Eval_item::is_graded_automatically() const {
  return is_informational() && relative_value() == 0;
}

std::ostream &operator<<(std::ostream &o, Eval_item::Type type) {
  switch (type) {
  case Eval_item::Type::Boolean:
    return o << "Boolean";
  case Eval_item::Type::Scale:
    return o << "Scale";
  case Eval_item::Type::Informational:
    return o << "Informational";
  }
}

char const *Enum<Eval_item::Type>::show(Eval_item::Type type) {
  using T = Eval_item::Type;

  switch (type) {
  case T::Boolean:
    return "boolean";
  case T::Scale:
    return "scale";
  case T::Informational:
    return "informational";
  }
}

namespace rc = std::regex_constants;

static std::regex const boolean_re("boolean", rc::icase);

static std::regex const scale_re("scale", rc::icase);

static std::regex const informational_re("informational", rc::icase);

Eval_item::Type Enum<Eval_item::Type>::read(const char *type) {
  using T = Eval_item::Type;

  auto match = [=](auto re) { return std::regex_match(type, re); };

  if (match(boolean_re))
    return T::Boolean;

  if (match(scale_re))
    return T::Scale;

  if (match(informational_re))
    return T::Informational;

  throw std::invalid_argument{"Could not parse eval item type"};
}

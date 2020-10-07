#include "../common/format.h"
#include "Abstract_evaluation.h"
#include "Eval_item.h"
#include "Submission.h"

#include <type_traits>

namespace J = Wt::Json;

double Abstract_evaluation::normalize_score(double score) {
  if (score < std::numeric_limits<double>::min()) {
    return 0;
  } else if (score > 1) {
    return 1;
  } else {
    return score;
  }
}

void Abstract_evaluation::set_explanation(const std::string &explanation) {
  explanation_ = explanation;
  touch_();
}

void Abstract_evaluation::set_score(double score) {
  score_ = normalize_score(score);
  submission()->light_touch();
  touch_();
}

const dbo::ptr<Assignment> &Abstract_evaluation::assignment() const {
  return submission()->assignment();
}

std::string Abstract_evaluation::plain_score_string() const {
  return eval_item()->format_score(score());
}

void Abstract_evaluation::touch_() { time_stamp_.touch(); }

Wt::Json::Object Abstract_evaluation::to_json() const {
  J::Object result;

  result["score"] = J::Value(clean_grade(score()));
  result["explanation"] = J::Value(explanation());

  return result;
}

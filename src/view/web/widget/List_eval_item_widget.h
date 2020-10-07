#pragma once

#include "Base_eval_item_widget.h"

// A list of evaluation items, which allows focusing on a particular item.
class List_eval_item_widget : public Base_eval_item_widget {
public:
  List_eval_item_widget(const Submission::Item &, Evaluation_view &, Session &);

private:
  void focus_action_();

  void add_scores_();
  void add_buttons_();
};

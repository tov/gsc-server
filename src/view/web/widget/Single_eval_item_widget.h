#pragma once

#include "Base_eval_item_widget.h"

class Single_eval_item_widget : public Base_eval_item_widget {
public:
  Single_eval_item_widget(const Submission::Item &, Evaluation_view &,
                          Session &);
};

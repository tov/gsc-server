#pragma once

#include "Single_eval_item_widget.h"

class Review_eval_item_widget : public Single_eval_item_widget {
public:
  Review_eval_item_widget(const Submission::Item &, Evaluation_view &,
                          Session &);
};

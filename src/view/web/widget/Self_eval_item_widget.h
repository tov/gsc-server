#pragma once

#include "Single_eval_item_widget.h"

#include <Wt/WPushButton.h>

class Response_widget;

class Self_eval_item_widget : public Single_eval_item_widget {
public:
  Self_eval_item_widget(const Submission::Item &, Evaluation_view &, Session &);

private:
  Response_widget *response_widget_;
  Wt::WPushButton *save_button_;
  Wt::WPushButton *back_button_;

  void save_action_();
  void back_action_();
  void validate_();
};

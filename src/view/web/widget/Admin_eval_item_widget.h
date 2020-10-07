#pragma once

#include "../../../model/Grader_eval.h"
#include "Single_eval_item_widget.h"

#include <Wt/WPushButton.h>

class Admin_response_widget;

class Admin_eval_item_widget : public Single_eval_item_widget {
public:
  Admin_eval_item_widget(const Submission::Item &, Evaluation_view &,
                         Session &);

  using This = Admin_eval_item_widget;

private:
  Admin_response_widget *self_response_;
  Wt::WPushButton *self_save_button_;

  void load_();
  void reload_();

  void self_update_button_();

  void self_save_action_();
  void self_retract_action_();

  Admin_response_widget *grader_response_;
  Wt::WPushButton *grader_save_button_;
  Wt::WPushButton *grader_hold_button_;

  void grader_update_button_();

  void grader_save_action_();
  void grader_hold_action_();
  void grader_retract_action_();

  void grader_save_status_(Grader_eval::Status);
};

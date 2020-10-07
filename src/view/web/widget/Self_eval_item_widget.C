#include "../view/Evaluation_view.h"
#include "Response_widget.h"
#include "Self_eval_item_widget.h"

#include "../../../Session.h"
#include "../../../model/Grader_eval.h"
#include "../../../model/Self_eval.h"

Self_eval_item_widget::Self_eval_item_widget(const Submission::Item &model,
                                             Evaluation_view &main,
                                             Session &session)
    : Single_eval_item_widget(model, main, session) {
  {
    auto response_widget = Response_widget::create(model.eval_item->type());
    response_widget_ = response_widget.get();
    addWidget(std::move(response_widget));
  }

  if (model.self_eval) {
    response_widget_->set_initial_value(model.self_eval->score());
    response_widget_->set_initial_explanation(model.self_eval->explanation());
  }

  response_widget_->changed().connect([=] { validate_(); });

  auto buttons = addNew<Wt::WContainerWidget>();
  buttons->setStyleClass("buttons");

  auto save_label = model.eval_item->is_informational() ? "Continue" : "Save";
  save_button_ = buttons->addNew<Wt::WPushButton>(save_label);
  back_button_ = buttons->addNew<Wt::WPushButton>("Back");
  save_button_->clicked().connect([=] { save_action_(); });
  back_button_->clicked().connect([=] { back_action_(); });

  if (model.self_eval && model.self_eval->frozen()) {
    save_button_->hide();
    response_widget_->freeze();
  } else {
    back_button_->hide();
    if (model.self_eval && model.self_eval->frozen_score())
      response_widget_->freeze_value();
  }

  if (model_.grader_eval) {
    auto score = model_.eval_item->format_score(model_.grader_eval->score());
    add_evaluation_("Grader evaluation <small>(old)</small>", score,
                    model_.grader_eval->explanation());
  }

  validate_();
}

void Self_eval_item_widget::save_action_() {
  if (!main_.can_eval())
    return;

  dbo::Transaction transaction(session_);
  auto self_eval =
      Submission::get_self_eval(model_.eval_item, main_.submission());
  Submission::save_self_eval(self_eval, session_, response_widget_->value(),
                             response_widget_->explanation());
  transaction.commit();

  main_.go_default();
}

void Self_eval_item_widget::back_action_() { main_.go_default(); }

void Self_eval_item_widget::validate_() {
  if (!save_button_)
    return;

  if (response_widget_->has_changed()) {
    back_button_->hide();
    save_button_->show();
    save_button_->setEnabled(response_widget_->is_valid());
  } else {
    save_button_->hide();
    back_button_->setHidden(!response_widget_->is_valid());
  }
}

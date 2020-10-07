#include "../../../Session.h"
#include "../../../model/Grader_eval.h"
#include "../../../model/Self_eval.h"
#include "../view/Evaluation_view.h"
#include "List_eval_item_widget.h"

#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

List_eval_item_widget::List_eval_item_widget(const Submission::Item &model,
                                             Evaluation_view &main,
                                             Session &session)
    : Base_eval_item_widget(model, main, session) {
  add_item_heading_();
  add_question_();
  add_scores_();
  add_buttons_();
}

void List_eval_item_widget::add_buttons_() {
  auto buttons = addNew<Wt::WContainerWidget>();
  buttons->setStyleClass("buttons");

  auto focus_btn = buttons->addNew<Wt::WPushButton>();

  if (main_.submission()->can_eval(session_.user()) &&
      !model_.eval_item->is_informational() &&
      !(model_.self_eval && model_.self_eval->frozen())) {
    focus_btn->setText("Edit");
  } else {
    focus_btn->setText("View");
  }

  focus_btn->clicked().connect(this, &List_eval_item_widget::focus_action_);
}

void List_eval_item_widget::add_scores_() {
  Viewing_context cxt{session_.user()};

  Score_owner self;
  Score_owner grader;

  Wt::WString attention_class = "list-eval-item";

  if (model_.self_eval) {
    self = model_.self_eval->score_owner(cxt);
  } else {
    self.score = "[not set]";
  }

  if (model_.grader_eval && model_.grader_eval->is_ready()) {
    grader = model_.grader_eval->score_owner(cxt);

    if (model_.grader_eval->score() < model_.self_eval->score())
      attention_class = "list-eval-item has-been-docked";
    else if (!model_.grader_eval->explanation().empty())
      attention_class = "list-eval-item has-explanation";
  }

  setStyleClass(attention_class);

  auto table = addNew<Wt::WTemplate>(
      model_.eval_item->is_informational()
          ? model_.eval_item->absolute_value() == 0
                ? ""
                : "<table class='scores'>"
                  "<tr><th>${grader}</th><td>${grader-score}</td></tr>"
                  "</table>"
          : "<table class='scores'>"
            "<tr><th>${self}</th><td>${self-score}</td></tr>"
            "<tr><th>${grader}</th><td>${grader-score}</td></tr>"
            "</table>");

  table->bindString("self", self.owner);
  table->bindString("self-score", self.score);
  table->bindString("grader", grader.owner);
  table->bindString("grader-score", grader.score);
}

void List_eval_item_widget::focus_action_() {
  main_.go_to((unsigned)model_.eval_item->sequence());
}

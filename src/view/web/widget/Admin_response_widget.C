#include "Admin_response_widget.h"
#include "Explanation_edit_widget.h"
#include "Unit_line_edit.h"

#include <Wt/WPushButton.h>

Admin_response_widget::Admin_response_widget(Abstract_evaluation *model) {
  auto impl = setNewImplementation<Wt::WContainerWidget>();
  impl->setStyleClass("admin-response-widget");

  explanation_ = impl->addNew<Explanation_edit_widget>();

  buttons_ = impl->addNew<Wt::WContainerWidget>();
  buttons_->setStyleClass("buttons");

  grade_ = buttons_->addNew<Unit_line_edit>();
  grade_->setStyleClass("unit-edit");

  using This = Admin_response_widget;
  explanation_->changed().connect(this, &This::handle_change_);
  grade_->valid().connect(this, &This::handle_change_);
  grade_->invalid().connect(this, &This::handle_change_);

  load(model);
}

void Admin_response_widget::load(const Abstract_evaluation *model) {
  set_model_(model);

  grade_->set_value(model_.score);
  if (model_.score != Unit_line_edit::INVALID)
    explanation_->setText(model_.explanation);
  else
    explanation_->setText("");
}

bool Admin_response_widget::save(Abstract_evaluation *model) {
  auto score = grade_->value();
  if (score == Unit_line_edit::INVALID)
    return false;

  // TODO: This doesn't give 10% for Boolean = 0.
  model->set_explanation(explanation_->text().toUTF8());
  model->set_score(score);

  set_model_(model);

  return true;
}

bool Admin_response_widget::is_valid() {
  return grade_->value() != Unit_line_edit::INVALID;
}

bool Admin_response_widget::is_saved() {
  if (model_.score == Unit_line_edit::INVALID)
    return false;

  return grade_->value() == model_.score &&
         explanation_->text() == model_.explanation;
}

void Admin_response_widget::handle_change_() { changed().emit(); }

void Admin_response_widget::setFocus(bool focus) {
  explanation_->setFocus(focus);
}

void Admin_response_widget::set_model_(const Abstract_evaluation *model) {
  if (model) {
    model_.explanation = model->explanation();
    model_.score = model->score();
  } else {
    model_.explanation = "";
    model_.score = Unit_line_edit::INVALID;
  }
}

void Admin_response_widget::load() { WCompositeWidget::load(); }

#include "Admin_response_widget.h"
#include "Unit_line_edit.h"
#include "Explanation_text_area.h"

#include <Wt/WPushButton>

Admin_response_widget::Admin_response_widget(Wt::WContainerWidget* parent)
        : WCompositeWidget(parent)
{
    auto impl = new Wt::WContainerWidget;
    setImplementation(impl);
    impl->setStyleClass("admin-response-widget");

    explanation_ = new Explanation_text_area(impl);

    buttons_ = new Wt::WContainerWidget(impl);
    buttons_->setStyleClass("buttons");

    grade_ = new Unit_line_edit(buttons_);
    grade_->setStyleClass("unit-edit");

    using This = Admin_response_widget;
    explanation_->changed().connect(this, &This::handle_change_);
    grade_->valid().connect(this, &This::handle_change_);
    grade_->invalid().connect(this, &This::handle_change_);
}

void Admin_response_widget::load(const Abstract_evaluation* model)
{
    model_ = model;

    if (model_) {
        explanation_->setText(model_->explanation());
        grade_->set_value(model_->score());
    } else {
        explanation_->setText("");
        grade_->set_value(Unit_line_edit::INVALID);
    }
}

bool Admin_response_widget::save(Abstract_evaluation* model)
{
    auto score = grade_->value();
    if (score == Unit_line_edit::INVALID) return false;

    model->set_explanation(explanation_->text().toUTF8());
    model->set_score(score);

    model_ = model;

    return true;
}

bool Admin_response_widget::is_valid()
{
    return grade_->value() != Unit_line_edit::INVALID;
}

bool Admin_response_widget::is_saved()
{
    return model_ &&
            grade_->value() == model_->score() &&
            explanation_->text() == model_->explanation();
}

void Admin_response_widget::handle_change_()
{
    changed().emit();
}

void Admin_response_widget::setFocus(bool focus)
{
    explanation_->setFocus(focus);
}



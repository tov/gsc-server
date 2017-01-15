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

    auto buttons = new Wt::WContainerWidget(impl);
    buttons->setStyleClass("buttons");

    grade_ = new Unit_line_edit(buttons);
    save_button_ = new Wt::WPushButton("Save", buttons);
    retract_button_ = new Wt::WPushButton("Retract", buttons);
}

void Admin_response_widget::load(const Abstract_evaluation* model)
{
    explanation_->setText(model->explanation());
    grade_->set_value(model->score());
}

bool Admin_response_widget::save(Abstract_evaluation* model)
{
    auto score = grade_->value();
    if (score == Unit_line_edit::INVALID) return false;

    model->set_explanation(explanation_->text().toUTF8());
    model->set_score(score);
    return true;
}

bool Admin_response_widget::is_valid()
{
    return grade_->value() != Unit_line_edit::INVALID;
}

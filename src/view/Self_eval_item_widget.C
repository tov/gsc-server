#include "Self_eval_item_widget.h"
#include "Response_widget.h"
#include "Evaluation_view.h"

#include "../model/Grader_eval.h"
#include "../model/Self_eval.h"
#include "../model/Session.h"

Self_eval_item_widget::Self_eval_item_widget(const Submission::Item& model, Evaluation_view& main,
                                             Session& session)
        : Single_eval_item_widget(model, main, session)
{
    auto response_widget = Response_widget::create(model.eval_item->type());
    response_widget_ = response_widget.get();
    addWidget(std::move(response_widget));

    if (model.self_eval) {
        response_widget_->set_value(model.self_eval->score());
        response_widget_->set_explanation(model.self_eval->explanation());
    }

    response_widget_->changed().connect(this,
                                        &Self_eval_item_widget::validate_);

    auto buttons = addNew<Wt::WContainerWidget>();
    buttons->setStyleClass("buttons");

    if (model.grader_eval && model.grader_eval->score() == 1) {
        response_widget_->setDisabled(true);
        save_button_ = buttons->addNew<Wt::WPushButton>("Back");
        save_button_->clicked().connect(this,
                                        &Self_eval_item_widget::back_action_);
    } else {
        auto label = model.eval_item->type() == Eval_item::Type::Informational
                     ? "Continue"
                     : "Save";
        save_button_ = buttons->addNew<Wt::WPushButton>(label);
        save_button_->clicked().connect(this,
                                        &Self_eval_item_widget::save_action_);
    }

    if (model_.grader_eval) {
        auto score = model_.eval_item->format_score(model_.grader_eval->score());
        add_evaluation_("Grader evaluation <small>(old)</small>",
                        score,
                        model_.grader_eval->explanation());
    }

    validate_();
}

void Self_eval_item_widget::save_action_()
{
    if (!main_.can_eval()) return;

    dbo::Transaction transaction(session_);
    auto self_eval = Submission::get_self_eval(model_.eval_item,
                                               main_.submission());
    Submission::save_self_eval(self_eval, session_,
                               response_widget_->value(),
                               response_widget_->explanation());
    transaction.commit();

    main_.go_default();
}

void Self_eval_item_widget::back_action_()
{
    main_.go_default();
}

void Self_eval_item_widget::validate_()
{
    save_button_->setEnabled(response_widget_->is_ready());
}


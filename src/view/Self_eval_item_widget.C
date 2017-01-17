#include "Self_eval_item_widget.h"
#include "Response_widget.h"
#include "Evaluation_view.h"

#include "../model/Grader_eval.h"
#include "../model/Self_eval.h"
#include "../model/Session.h"

Self_eval_item_widget::Self_eval_item_widget(
        const Submission::Item& model,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Single_eval_item_widget(model, main, session, parent)
{
    response_widget_ = Response_widget::create(model.eval_item->type(),
                                               this);
    if (model.self_eval) {
        response_widget_->set_value(model.self_eval->score());
        response_widget_->set_explanation(model.self_eval->explanation());
    }

    response_widget_->changed().connect(this,
                                        &Self_eval_item_widget::validate_);

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");
    auto label = model.eval_item->type() == Eval_item::Type::Informational
                 ? "Continue" : "Save";
    save_button_ = new Wt::WPushButton(label, buttons);

    save_button_->clicked().connect(this,
                                    &Self_eval_item_widget::save_action_);

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

void Self_eval_item_widget::validate_()
{
    save_button_->setEnabled(response_widget_->is_ready());
}


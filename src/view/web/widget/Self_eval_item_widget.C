#include "Self_eval_item_widget.h"
#include "Response_widget.h"
#include "../view/Evaluation_view.h"

#include "../../../model/Grader_eval.h"
#include "../../../model/Self_eval.h"
#include "../../../Session.h"

namespace {

char const* save_label(Submission::Item const& m, bool changed = false)
{
    if (m.eval_item->is_informational())
        return "Okay";

    if (changed || !m.self_eval)
        return "Save";

    return "[Saved]";
}

char const* back_label(Submission::Item const& m)
{
    if (m.fully_frozen()) {
        return "Back";
    } else {
        return "Cancel";
    }
}

}  // anonymous namespace

Self_eval_item_widget::Self_eval_item_widget(
        const Submission::Item& model,
        Evaluation_view& main,
        Session& session)
        : Single_eval_item_widget(model, main, session)
{
    {
        auto response_widget = Response_widget::create(model.eval_item->type());
        response_widget_ = response_widget.get();
        addWidget(std::move(response_widget));
    }

    if (model.self_eval) {
        response_widget_->set_initial_value(model.self_eval->score());
        response_widget_->set_initial_explanation(model.self_eval->explanation());
    }

    response_widget_->changed().connect([=]{ validate_(); });

    if (model.fully_frozen()) {
        response_widget_->freeze();
    } else if (model.score_frozen()) {
        response_widget_->freeze_value();
    }

    auto buttons = addNew<Wt::WContainerWidget>();
    buttons->setStyleClass("buttons");

    back_button_ = buttons->addNew<Wt::WPushButton>(back_label(model_));
    back_button_->clicked().connect([=] { back_action_(); });
    back_button_->setStyleClass("btn");

    save_button_ = buttons->addNew<Wt::WPushButton>(save_label(model_));
    save_button_->clicked().connect([=] { save_action_(); });
    save_button_->setStyleClass("btn btn-primary");

    if (model.self_eval) {
        if (model.eval_item->is_informational() || model.fully_frozen()) {
            save_button_->hide();
        }
    } else {
        back_button_->hide();
    }

    Viewing_context cxt{session_.user()};

    if (auto view = model.view_grader_eval(cxt, "")) {
        add_evaluation_("Grader evaluation <small>(previous)</small>",
                        view->score,
                        view->explanation);
    }

    validate_();
}

void Self_eval_item_widget::save_action_()
{
    if (!main_.can_eval()) return;

    dbo::Transaction transaction(session_);
    auto self_eval = Submission::get_self_eval(model_.eval_item,
                                               main_.submission());
    Submission::save_self_eval(self_eval,
                               session_.user(),
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
    dbo::Transaction transaction(session_);

    back_button_->setText(back_label(model_));

    bool changed = response_widget_->has_changed();
    save_button_->setText(save_label(model_, changed));
    save_button_->setEnabled(changed && response_widget_->is_valid());
}


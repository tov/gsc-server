#include "List_eval_item_widget.h"
#include "../view/Evaluation_view.h"
#include "../../../model/Self_eval.h"
#include "../../../model/Grader_eval.h"
#include "../../../Session.h"
#include "../../../common/util.h"

#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

List_eval_item_widget::List_eval_item_widget(const Submission::Item& model, Evaluation_view& main,
                                             Session& session)
        : Base_eval_item_widget(model, main, session)
{
    add_item_heading_();
    add_question_();
    add_scores_();
    add_buttons_();
}

static WString
attention_class(dbo::ptr<Grader_eval> const& grader_eval)
{
    if (!grader_eval || !grader_eval->is_ready())
        return "list-eval-item";
    else if (grader_eval->score() < 1)
        return "list-eval-item has-been-docked";
    else if (! grader_eval->explanation().empty())
        return "list-eval-item has-explanation";
    else
        return "list-eval-item has-been-graded";
}

static WString
focus_btn_class(dbo::ptr<Grader_eval> const& grader_eval,
                bool can_edit)
{
    if (!grader_eval || !grader_eval->is_ready())
        return can_edit ? "btn btn-success" : "btn";
    else if (grader_eval->score() < 1)
        return can_edit ? "btn btn-primary" : "btn btn-info";
    else if (! grader_eval->explanation().empty())
        return "btn btn-info";
    else
        return "btn";
}

void List_eval_item_widget::add_buttons_()
{
    auto buttons = addNew<WContainerWidget>();
    buttons->setStyleClass("buttons");

    auto focus_btn = buttons->addNew<WPushButton>();

    if (main_.submission()->can_eval(session_.user())
        && !model_.eval_item->is_informational()
        && !(model_.self_eval && model_.self_eval->frozen())) {
        focus_btn->setText("Edit");
        focus_btn->setStyleClass(focus_btn_class(model_.grader_eval, true));
    } else {
        focus_btn->setText("View");
        focus_btn->setStyleClass(focus_btn_class(model_.grader_eval, false));
    }

    focus_btn->clicked().connect(this, &List_eval_item_widget::focus_action_);
}

void List_eval_item_widget::add_scores_()
{
    Viewing_context cxt {session_.user()};

    Score_owner self;
    Score_owner grader;

    if (model_.self_eval) {
        self = model_.self_eval->score_owner(cxt);
    } else {
        self.score = "[not set]";
    }

    setStyleClass(attention_class(model_.grader_eval));

    auto table = addNew<WTemplate>(
            model_.eval_item->is_informational()
            ? model_.eval_item->absolute_value() == 0
              ? ""
              : "<table class='scores'>"
                  "<tr><th>${grader}</th><td>${grader-score}</td></tr>"
                "</table>"
            : "<table class='scores'>"
                "<tr><th>${self}</th><td>${self-score}</td></tr>"
                "<tr><th>${grader}</th><td>${grader-score}</td></tr>"
              "</table>"
    );

    table->bindString("self", self.owner);
    table->bindString("self-score", self.score);
    table->bindString("grader", grader.owner);
    table->bindString("grader-score", grader.score);
}

void List_eval_item_widget::focus_action_()
{
    main_.go_to((unsigned) model_.eval_item->sequence());
}


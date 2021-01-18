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
attention_class(Submission::Item const& m)
{
    ostringstream buf;

    buf << "list-eval-item";

    if (m.grader_eval && m.grader_eval->is_ready()) {
        buf << " has-been-graded";

        if (m.grader_eval->score() < m.self_eval->score())
            buf << " has-been-docked";

        if (! m.grader_eval->explanation().empty())
            buf << " has-explanation";
    }

    return buf.str();
}

static WString
focus_btn_class(Submission::Item const& m, bool can_edit)
{
    if (!m.grader_eval || !m.grader_eval->is_ready())
        return can_edit ? "btn btn-success" : "btn";
    else if (m.grader_eval->score() < m.self_eval->score())
        return can_edit ? "btn btn-warning" : "btn btn-danger";
    else if (! m.grader_eval->explanation().empty())
        return "btn btn-info";
    else
        return "btn";
}

void List_eval_item_widget::add_buttons_()
{
    auto buttons = addNew<WContainerWidget>();
    buttons->setStyleClass("buttons");

    auto focus_btn = buttons->addNew<WPushButton>();

    if (!main_.submission()->can_eval(session_.user()) ||
            (model_.self_eval && model_.self_eval->fully_frozen())) {
        focus_btn->setText("View");
        focus_btn->setStyleClass(focus_btn_class(model_, false));
    } else {
        focus_btn->setText("Edit");
        focus_btn->setStyleClass(focus_btn_class(model_, true));
    }

    focus_btn->clicked().connect(this, &List_eval_item_widget::focus_action_);
}

void List_eval_item_widget::add_scores_()
{
    Viewing_context cxt {session_.user()};

    Score_owner self;
    Score_owner grader;

    if (!model_.self_eval) {
        self.owner = "self evaluation";
        self.score = "[not set]";
    } else if (!model_.eval_item->is_graded_automatically()) {
        self = model_.self_eval->score_owner(cxt);

        if (!model_.grader_eval) {
            grader.score = "[not set]";
        } else {
            grader = model_.grader_eval->score_owner(cxt);
        }

        if (grader.owner.empty()) {
            grader.owner = "grader";
        }
    }

    setStyleClass(attention_class(model_));

    auto table = addNew<WTemplate>(
            "<table class='scores'>"
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


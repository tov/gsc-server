#include "List_eval_item_widget.h"
#include "Evaluation_view.h"
#include "../model/Self_eval.h"
#include "../model/Grader_eval.h"
#include "../model/Session.h"

#include <Wt/WPushButton>
#include <Wt/WTemplate>

List_eval_item_widget::List_eval_item_widget(
        const Submission::Item& model,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Base_eval_item_widget(model, main, session, parent)
{
    add_item_heading_();
    add_question_();
    add_scores_();
    add_buttons_();
}

void List_eval_item_widget::add_buttons_()
{
    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");

    auto focus_btn = new Wt::WPushButton(buttons);

    if (main_.submission()->can_eval(session_.user())) {
        focus_btn->setText("Edit");
    } else {
        focus_btn->setText("View");
    }

    focus_btn->clicked().connect(this, &List_eval_item_widget::focus_action_);
}

void List_eval_item_widget::add_scores_()
{
    auto current_user = session_.user();
    auto role = current_user->role();

    Wt::WString self;
    Wt::WString self_score;
    Wt::WString grader;
    Wt::WString grader_score;

    switch (role) {
        case User::Role::Student:
            self = "You";
            break;
        case User::Role::Grader:
            self = "Student";
            break;
        case User::Role::Admin:
            self = main_.submission()->owner_string();
    }

    self_score = model_.self_eval
                 ? model_.self_eval->score_string()
                 : "[not set]";

    if (main_.submission()->is_graded() ||
        (role == User::Role::Admin && model_.grader_eval))
    {
        auto grader_user = model_.grader_eval->grader();
        grader = grader_user->can_grade() ? grader_user->name() : "Auto";
        grader_score = model_.grader_eval->score_string();
    } else {
        grader = "Grader";
        grader_score = "[not set]";
    }

    auto table = new Wt::WTemplate(
            "<table class='scores'>"
                    "<tr><th>${self}</th><td>${self-score}</td></tr>"
                    "<tr><th>${grader}</th><td>${grader-score}</td></tr>"
                    "</table>",
            this
    );

    table->bindWidget("self", new Wt::WText(self));
    table->bindWidget("self-score", new Wt::WText(self_score));
    table->bindWidget("grader", new Wt::WText(grader));
    table->bindWidget("grader-score", new Wt::WText(grader_score));
}

void List_eval_item_widget::focus_action_()
{
    main_.go_to((unsigned) model_.eval_item->sequence());
}


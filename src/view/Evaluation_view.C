#include "Evaluation_view.h"
#include "Base_eval_item_widget.h"
#include "Eval_widget.h"
#include "Widget_factory.h"
#include "File_viewer_widget.h"
#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../model/Eval_item.h"
#include "../model/Grader_eval.h"
#include "../model/Self_eval.h"
#include "../model/Session.h"
#include "../model/Submission.h"

#include <Wt/WApplication>
#include <Wt/WHBoxLayout>
#include <Wt/WPushButton>
#include <Wt/WTemplate>
#include <Wt/WText>

#include <iomanip>

class List_eval_item_widget : public Base_eval_item_widget
{
public:
    List_eval_item_widget(const Submission::Item&,
                          Evaluation_view&,
                          Session&,
                          Wt::WContainerWidget* parent = nullptr);

private:
    void focus_action_();

    void add_scores_();
    void add_buttons_();
};

Evaluation_view::Evaluation_view(const dbo::ptr<Submission>& submission,
                                 Session& session,
                                 Wt::WContainerWidget* parent)
        : Abstract_file_view(submission, session, parent)
{
    load_();
    setStyleClass("evaluation-view");
}

void Evaluation_view::load_()
{
    dbo::Transaction transaction(session_);
    submission_->reload_cache();
}

void Evaluation_view::go_to(unsigned int index)
{
    std::ostringstream path;
    path << submission_->eval_url();
    path << '/' << index;
    Wt::WApplication::instance()->setInternalPath(path.str());

    right_column_->clear();
    Eval_widget::create(submission_->items().at(index), *this, session_,
                        right_column_);
}

void Evaluation_view::go_default()
{
    if (session_.user()->role() == User::Role::Student && can_eval()) {
        for (auto& row : submission_->items()) {
            if (row.eval_item && !row.self_eval) {
                go_to((unsigned int) row.eval_item->sequence());
                return;
            }
        }
    }

    Wt::WApplication::instance()->setInternalPath(submission_->eval_url());

    right_column_->clear();
    for (auto& row : submission_->items()) {
        if (!row.eval_item) continue;

        new List_eval_item_widget(row, *this, session_, right_column_);
    }
}

bool Evaluation_view::can_eval()
{
    return submission_->can_eval(session_.user());
}

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

    if (main_.submission()->can_eval(session_.user())) {
        auto edit = new Wt::WPushButton("Edit", buttons);
        edit->clicked().connect(this,
                                &List_eval_item_widget::focus_action_);

        if (session_.user()->can_admin()) {
            auto retract = new Wt::WPushButton("Retract", buttons);
            retract->clicked().connect(this,
                                       &List_eval_item_widget::retract_action_);
        }
    } else {
        auto view = new Wt::WPushButton("View", buttons);
        view->clicked().connect(this,
                                &List_eval_item_widget::focus_action_);
    }
}

void List_eval_item_widget::add_scores_()
{
    auto role = session_.user()->role();

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
            ? format_score_(model_.eval_item->type(),
                            model_.self_eval->score())
            : "<em>[not set]</em>";

    if (main_.submission()->is_graded() ||
        (role == User::Role::Admin && model_.grader_eval))
    {
        grader = model_.grader_eval->grader()->name();
        grader_score = format_score_(model_.eval_item->type(),
                                     model_.grader_eval->score());
    } else {
        grader = "Grader";
        grader_score = "<em>[not set]</em>";
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


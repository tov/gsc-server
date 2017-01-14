#include "Evaluation_view.h"
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

class Evaluation_list_view_item : public Wt::WContainerWidget
{
public:
    Evaluation_list_view_item(const Submission::Item&,
                              Evaluation_view*,
                              Session&,
                              Wt::WContainerWidget* parent = nullptr);

private:
    const Submission::Item& model_;
    Evaluation_view* const main_;
    Session& session_;

    void focus_action_();
    void retract_action_();

    void add_item_heading_();
    void add_question_();
    void add_scores_();
    void add_buttons_();

    std::string format_score_(const Eval_item::Type& type, double score) const;
};

Evaluation_view::Evaluation_view(const dbo::ptr<Submission>& submission,
                                 Session& session,
                                 Wt::WContainerWidget* parent)
        : Abstract_file_view(submission, session, parent)
        , role_(session.user()->role())
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

    rows_.clear();
    rows_.push_back(Eval_widget::create(submission_->items().at(index), true,
                                        *this, session_, right_column_));
}

void Evaluation_view::go_default()
{
    rows_.clear();

    if (role_ == User::Role::Student && can_eval_()) {
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

        new Evaluation_list_view_item(row, this, session_, right_column_);
    }
}

bool Evaluation_view::can_eval_()
{
    return submission_->can_eval(session_.user());
}

Evaluation_list_view_item::Evaluation_list_view_item(
        const Submission::Item& model,
        Evaluation_view* main,
        Session& session,
        Wt::WContainerWidget* parent)
        : WContainerWidget(parent)
        , model_(model)
        , main_(main)
        , session_(session)
{
    add_item_heading_();
    add_question_();
    add_scores_();
    add_buttons_();
}

void Evaluation_list_view_item::add_buttons_()
{
    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");

    if (main_->submission()->can_eval(session_.user())) {
        auto edit = new Wt::WPushButton("Edit", buttons);
        edit->clicked().connect(this,
                                &Evaluation_list_view_item::focus_action_);

        if (session_.user()->can_admin()) {
            auto retract = new Wt::WPushButton("Retract", buttons);
            retract->clicked().connect(this,
                                       &Evaluation_list_view_item::retract_action_);
        }
    } else {
        auto view = new Wt::WPushButton("View", buttons);
        view->clicked().connect(this,
                                &Evaluation_list_view_item::focus_action_);
    }
}

void Evaluation_list_view_item::add_scores_()
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
            self = main_->submission()->owner_string();
    }

    self_score = model_.self_eval
            ? format_score_(model_.eval_item->type(),
                            model_.self_eval->score())
            : "<em>[not set]</em>";

    if (main_->submission()->is_graded() ||
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

namespace {

inline std::string pct_string(double ratio)
{
    std::ostringstream fmt;
    fmt << std::setprecision(2) << 100 * ratio << '%';
    return fmt.str();
}

}

void Evaluation_list_view_item::add_item_heading_()
{
    auto h4 = new Wt::WTemplate("<h4>Question ${number} "
                                        "<small>${value}</small></h4>",
                                this);

    std::string number = boost::lexical_cast<std::string>(
            model_.eval_item->sequence());
    std::string value = pct_string(model_.eval_item->relative_value()
                                   / main_->submission()->point_value());

    h4->bindWidget("number", new Wt::WText(number));
    h4->bindWidget("value", new Wt::WText(value));
}

void Evaluation_list_view_item::add_question_()
{
    auto p = new Wt::WTemplate("<p>${question}</p>", this);
    p->bindWidget("question", new Wt::WText(model_.eval_item->prompt()));
}

std::string Evaluation_list_view_item::format_score_(
        const Eval_item::Type& type, double score) const
{
    if ((score != 0 && score != 1) || type == Eval_item::Type::Scale) {
        return pct_string(score);
    } else if (type == Eval_item::Type::Boolean) {
        if (score == 1.0) return "Yes";
        else return "No";
    } else {
        return "Okay";
    }
}

void Evaluation_list_view_item::focus_action_()
{
    main_->go_to((unsigned) model_.eval_item->sequence());
}

void Evaluation_list_view_item::retract_action_()
{
    dbo::Transaction transaction(session_);
    Submission::retract_self_eval(model_.self_eval);
    transaction.commit();

    main_->go_default();
}


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
    void add_scores_();
    void add_buttons_();

    void format_score_(const Eval_item::Type& type, double score,
                       std::ostringstream& fmt) const;
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

    std::ostringstream fmt;

    fmt << "<table>";
    fmt << "<tr><td>";

    switch (role) {
            case User::Role::Student:
                fmt << "You";
                break;
            case User::Role::Grader:
                fmt << "Student";
                break;
            case User::Role::Admin:
                fmt << main_->submission()->owner_string();
                break;
        }

    fmt << "</td><td>";
    if (!model_.self_eval)
            fmt << "<em>[not set]</em>";
        else
            format_score_(model_.eval_item->type(), model_.self_eval->score(), fmt);

    fmt << "</td></tr>";

    if (main_->submission()->is_graded() ||
        (role == User::Role::Admin && model_.grader_eval))
    {
        fmt << "<tr><td>";
        fmt << model_.grader_eval->grader();
        fmt << "</td><td>";
        format_score_(Eval_item::Type::Scale, model_.grader_eval->score(), fmt);
        fmt << "</td></tr>";
    }

    fmt << "</table>";

    new Wt::WText(fmt.str(), this);
}

void Evaluation_list_view_item::add_item_heading_()
{
    auto pct = 100 * model_.eval_item->relative_value()
               / main_->submission()->point_value();

    std::ostringstream fmt;
    fmt << "<h4>Question " << model_.eval_item->sequence() << " <small>(";
    fmt << std::setprecision(2) << pct << "%)</small></h4>";
    new Wt::WText(fmt.str(), this);
}

void Evaluation_list_view_item::format_score_(
        const Eval_item::Type& type, double score,
        std::ostringstream& fmt) const
{
    switch (type) {
        case Eval_item::Type::Boolean:
            if (score == 1.0)
                fmt << "Yes";
            else
                fmt << "No";
            break;
        case Eval_item::Type::Scale:
            fmt << std::setprecision(2) << 100 * score << '%';
            break;
        case Eval_item::Type::Informational:
            fmt << "Yes";
            break;
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


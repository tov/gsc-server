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
#include <Wt/WText>

#include <iomanip>

class Evaluation_list_view_item : public Wt::WContainerWidget
{
public:
    Evaluation_list_view_item(const Submission::Item&,
                              const dbo::ptr<Submission>&,
                              User::Role,
                              Wt::WContainerWidget* parent = nullptr);

private:
    const Submission::Item& model_;
    dbo::ptr<Submission> submission_;
    User::Role role_;

    void format_score_(const Eval_item::Type& type, double score,
                       std::ostringstream& fmt) const;

    void add_item_heading_();
    void add_scores_();
};

Evaluation_view::Evaluation_view(const dbo::ptr<Submission>& submission,
                                 Session& session,
                                 Wt::WContainerWidget* parent)
        : Abstract_file_view(submission, session, parent),
          role_(session.user()->role())
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

        new Evaluation_list_view_item(row, submission_, role_, right_column_);
    }
}

bool Evaluation_view::can_eval_()
{
    return submission_->can_eval(session_.user());
}

Evaluation_list_view_item::Evaluation_list_view_item(
        const Submission::Item& model,
        const dbo::ptr<Submission>& submission,
        User::Role role,
        Wt::WContainerWidget* parent)
        : WContainerWidget(parent)
        , model_(model)
        , submission_(submission)
        , role_(role)
{
    add_item_heading_();
    add_scores_();
}

void Evaluation_list_view_item::add_scores_()
{
    std::ostringstream fmt;

    fmt << "<table>";
    fmt << "<tr><td>";

    switch (role_) {
            case User::Role::Student:
                fmt << "You";
                break;
            case User::Role::Grader:
                fmt << "Student";
                break;
            case User::Role::Admin:
                fmt << submission_->owner_string();
                break;
        }

    fmt << "</td><td>";
    if (!model_.self_eval)
            fmt << "<em>[not set]</em>";
        else
            format_score_(model_.eval_item->type(), model_.self_eval->score(), fmt);

    fmt << "</td></tr>";

    if (submission_->is_graded() ||
        (role_ == User::Role::Admin && model_.grader_eval))
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
               / submission_->point_value();

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


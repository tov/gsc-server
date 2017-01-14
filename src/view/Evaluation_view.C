#include "Evaluation_view.h"
#include "Eval_widget.h"
#include "Widget_factory.h"
#include "File_viewer_widget.h"
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

Evaluation_view::Evaluation_view(const dbo::ptr<Submission>& submission,
                                 Session& session,
                                 Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          submission_(submission),
          session_(session),
          role_(session.user()->role())
{
    load_();

    setStyleClass("evaluation-view");

    auto hbox = new Wt::WHBoxLayout();
    setLayout(hbox);

    auto viewer_ = new File_viewer_widget(submission_, session_);
    hbox->addWidget(viewer_);

    right_column_ = new Wt::WContainerWidget;
    hbox->addWidget(right_column_, 1);
    right_column_->setStyleClass("right-column");
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

        auto item_widget =  new Wt::WContainerWidget(right_column_);
        add_item_heading_(row, item_widget);
        add_scores_(row, item_widget);
    }
}

void Evaluation_view::add_scores_(const Submission::Item& row,
                                  Wt::WContainerWidget* item_widget) const
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
    if (!row.self_eval)
            fmt << "<em>[not set]</em>";
        else
            format_score_(row.eval_item->type(), row.self_eval->score(), fmt);

    fmt << "</td></tr>";

    if (submission_->is_graded() ||
        (role_ == User::Role::Admin && row.grader_eval))
    {
        fmt << "<tr><td>";
        fmt << row.grader_eval->grader();
        fmt << "</td><td>";
        format_score_(Eval_item::Type::Scale, row.grader_eval->score(), fmt);
        fmt << "</td></tr>";
    }

    fmt << "</table>";

    new Wt::WText(fmt.str(), item_widget);
}

void Evaluation_view::add_item_heading_(const Submission::Item& row,
                                        Wt::WContainerWidget* item_widget) const
{
    auto pct = 100 * row.eval_item->relative_value()
               / submission_->point_value();

    std::__1::ostringstream fmt;
    fmt << "<h4>Question " << row.eval_item->sequence() << " <small>(";
    fmt << std::__1::setprecision(2) << pct << "%)</small></h4>";
    new Wt::WText(fmt.str(), item_widget);
}

void Evaluation_view::format_score_(const Eval_item::Type& type, double score,
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

bool Evaluation_view::can_eval_()
{
    return submission_->can_eval(session_.user());
}

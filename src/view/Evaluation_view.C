#include "Evaluation_view.h"
#include "Eval_widget.h"
#include "File_viewer_widget.h"
#include "../model/Assignment.h"
#include "../model/Eval_item.h"
#include "../model/Grader_eval.h"
#include "../model/Self_eval.h"
#include "../model/Session.h"
#include "../model/Submission.h"

#include <Wt/WApplication>
#include <Wt/WHBoxLayout>

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

    total_value_ = 0;

    for (const auto& eval_item : submission_->assignment()->eval_items()) {
        total_value_ += eval_item->relative_value();
        auto sequence = eval_item->sequence();
        while (model_.size() <= sequence) model_.emplace_back();
        model_[sequence].eval_item = eval_item;
    }

    for (const auto& self_eval : submission_->self_evals()) {
        auto sequence = self_eval->eval_item()->sequence();
        model_[sequence].self_eval = self_eval;
        model_[sequence].grader_eval = self_eval->grader_eval();
    }
}

void Evaluation_view::go_to(unsigned int index)
{
    std::ostringstream path;
    if (role_ != User::Role::Student) path << "/~" << submission_->user1()->name();
    path << "/hw/" << submission_->assignment()->number()
         << "/eval/" << index;
    Wt::WApplication::instance()->setInternalPath(path.str());

    Eval_widget::Mode mode;
    switch (role_) {
        case User::Role::Student:
            mode = Eval_widget::Mode::self_eval;
            break;
        case User::Role::Grader:
            mode = Eval_widget::Mode::grader_eval;
            break;
        case User::Role::Admin:
            mode = Eval_widget::Mode::admin_view;
            break;
    }

    rows_.clear();
    rows_.push_back(Eval_widget::create(model_.at(index), mode,
                                        *this, session_, right_column_));
}

void Evaluation_view::go_default()
{
    if (role_ == User::Role::Student && can_eval_()) {
        for (auto& row : model_) {
            if (row.eval_item && !row.self_eval) {
                go_to((unsigned int) row.eval_item->sequence());
                return;
            }
        }
    }

    std::ostringstream path;
    if (role_ != User::Role::Student) path << "/~" << submission_->user1()->name();
    path << "/hw/" << submission_->assignment()->number() << "/eval";
    Wt::WApplication::instance()->setInternalPath(path.str());

    Eval_widget::Mode mode;
    switch (role_) {
        case User::Role::Student:
            mode = Eval_widget::Mode::self_view;
            break;
        case User::Role::Grader:
            mode = Eval_widget::Mode::grader_eval;
            break;
        case User::Role::Admin:
            mode = Eval_widget::Mode::admin_view;
            break;
    }

    rows_.clear();
    for (auto& row : model_)
        if (row.eval_item)
            rows_.push_back(Eval_widget::create(row, mode,
                                                *this, session_, right_column_));
}

bool Evaluation_view::can_eval_()
{
    return submission_->can_eval(session_.user());
}

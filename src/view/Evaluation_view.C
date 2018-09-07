#include "Evaluation_view.h"

#include "Admin_eval_item_widget.h"
#include "List_eval_item_widget.h"
#include "Review_eval_item_widget.h"
#include "Self_eval_item_widget.h"
#include "Submission_owner_widget.h"

#include "File_viewer_widget.h"

#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../model/Eval_item.h"
#include "../model/Grader_eval.h"
#include "../model/Self_eval.h"
#include "../Session.h"
#include "../model/Submission.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

Evaluation_view::Evaluation_view(const Wt::Dbo::ptr<Submission>& submission,
                                 Session& session)
        : Abstract_file_view(submission, session)
{
    load_();
    setStyleClass("evaluation-view");
}

void Evaluation_view::load_()
{
    dbo::Transaction transaction(session_);
    submission_->load_cache();
}

void Evaluation_view::go_to(unsigned int index)
{
    std::ostringstream eval_url;
    eval_url << submission_->eval_url();
    eval_url << '/' << index;
    Wt::WApplication::instance()->setInternalPath(eval_url.str());

    right_column_->clear();

    const auto& model = submission_->items().at(index);

    if (session_.user()->can_admin())
        right_column_->addNew<Admin_eval_item_widget>(model, *this, session_);
    else if (submission_->can_eval(session_.user()))
        right_column_->addNew<Self_eval_item_widget>(model, *this, session_);
    else
        right_column_->addNew<Review_eval_item_widget>(model, *this, session_);
}

void Evaluation_view::go_default()
{
    dbo::Transaction transaction(session_);

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

    if (submission_->is_graded()) {
        std::ostringstream fmt;
        fmt << "<h2 class='submission-grade'>Grade: "
            << submission_->grade_string() << "</h2>";
        right_column_->addNew<Wt::WText>(fmt.str());
    }

    right_column_->addNew<Submission_owner_widget>(submission_, session_);

    for (auto& row : submission_->items()) {
        if (!row.eval_item) continue;

        right_column_->addNew<List_eval_item_widget>(row, *this, session_);
    }
}

bool Evaluation_view::can_eval()
{
    return submission_->can_eval(session_.user());
}


#include "Evaluation_view.h"

#include "../widget/Admin_eval_item_widget.h"
#include "../widget/List_eval_item_widget.h"
#include "../widget/Review_eval_item_widget.h"
#include "../widget/Self_eval_item_widget.h"
#include "../widget/Submission_owner_widget.h"

#include "../widget/File_viewer.h"

#include "../../../Session.h"
#include "../../../model/Assignment.h"
#include "../../../model/Eval_item.h"
#include "../../../model/Grader_eval.h"
#include "../../../model/Self_eval.h"
#include "../../../model/Submission.h"
#include "../../../model/auth/User.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

Evaluation_view::Evaluation_view(const Wt::Dbo::ptr<Submission> &submission,
                                 Session &session)
    : File_view_base(submission, session) {
  load_();
  setStyleClass("evaluation-view");
}

void Evaluation_view::load_() {
  dbo::Transaction transaction(session());
  submission()->load_cache();
}

void Evaluation_view::go_to(unsigned int index) {
  dbo::Transaction transaction(session());

  std::ostringstream eval_url;
  eval_url << submission()->url_for_user(principal(), true);
  eval_url << '/' << index;
  Wt::WApplication::instance()->setInternalPath(eval_url.str());

  right_column_->clear();

  const auto &model = submission()->items().at(index);

  if (session().user()->can_admin())
    right_column_->addNew<Admin_eval_item_widget>(model, *this, session());
  else if (submission()->can_eval(session().user()))
    right_column_->addNew<Self_eval_item_widget>(model, *this, session());
  else
    right_column_->addNew<Review_eval_item_widget>(model, *this, session());
}

void Evaluation_view::go_default() {
  dbo::Transaction transaction(session());

  if (session().user()->role() == User::Role::Student && can_eval()) {
    for (auto &row : submission()->items()) {
      if (row.eval_item && !row.self_eval) {
        go_to((unsigned int)row.eval_item->sequence());
        return;
      }
    }
  }

  Wt::WApplication::instance()->setInternalPath(
      submission()->url_for_user(principal(), true));

  right_column_->clear();

  if (submission()->is_graded()) {
    std::ostringstream fmt;
    fmt << "<h2 class='submission-grade'>Grade: "
        << submission()->grade_string() << "</h2>";
    right_column_->addNew<Wt::WText>(fmt.str());
  }

  right_column_->addNew<Submission_owner_widget>(submission(), session(),
                                                 changed());

  for (auto &row : submission()->items()) {
    if (!row.eval_item)
      continue;

    right_column_->addNew<List_eval_item_widget>(row, *this, session());
  }
}

bool Evaluation_view::can_eval() {
  return submission()->can_eval(session().user());
}

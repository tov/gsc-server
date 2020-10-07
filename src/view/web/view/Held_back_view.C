#include "../../../Navigate.h"
#include "../../../Session.h"
#include "../../../model/Assignment.h"
#include "../../../model/Eval_item.h"
#include "../../../model/Self_eval.h"
#include "Held_back_view.h"

#include <Wt/WAnchor.h>
#include <Wt/WLink.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

class Self_eval_table : public Wt::WContainerWidget {
public:
  explicit Self_eval_table(const dbo::collection<dbo::ptr<Self_eval>> &);
};

Self_eval_table::Self_eval_table(
    const dbo::collection<dbo::ptr<Self_eval>> &model) {
  auto table = addNew<Wt::WTable>();

  for (dbo::ptr<Self_eval> self_eval : model) {
    auto row = table->rowAt(table->rowCount());

    auto eval_url = self_eval->eval_url();
    auto grade_url = self_eval->grade_url();
    auto grader = self_eval->grader_eval()->grader()->name();

    row->elementAt(0)->addNew<Wt::WAnchor>(
        Wt::WLink(Wt::LinkType::InternalPath, eval_url), eval_url);
    row->elementAt(1)->addNew<Wt::WText>(grader);
    row->elementAt(2)->addNew<Wt::WAnchor>(
        Wt::WLink(Wt::LinkType::InternalPath, grade_url), grade_url);
  }
}

Held_back_view::Held_back_view(Session &session) : session_(session) {
  auto container = setNewImplementation<Wt::WContainerWidget>();
  setStyleClass("held-back");

  dbo::Transaction transaction(session_);

  {
    auto held_back = Self_eval::find_with_grade_status(
        Grader_eval::Status::held_back, session_);
    if (!held_back.empty()) {
      container->addNew<Wt::WText>("<h3>Held back</h3>");
      container->addNew<Self_eval_table>(held_back);
    }
  }

  {
    auto editing = Self_eval::find_with_grade_status(
        Grader_eval::Status::editing, session_);
    if (!editing.empty()) {
      container->addNew<Wt::WText>("<h3>Editing</h3>");
      container->addNew<Self_eval_table>(editing);
    }
  }
}

#include "Notable_self_evals_view.h"
#include "../../../Navigate.h"
#include "../../../model/Assignment.h"
#include "../../../model/Eval_item.h"
#include "../../../model/Self_eval.h"
#include "../../../Session.h"

#include <Wt/WAnchor.h>
#include <Wt/WLink.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

class Self_eval_table : public Wt::WContainerWidget
{
public:
    explicit Self_eval_table(const dbo::collection<dbo::ptr<Self_eval>>&);
};

Self_eval_table::Self_eval_table(const dbo::collection<dbo::ptr<Self_eval>>& model)
{
    auto table = addNew<Wt::WTable>();

    for (dbo::ptr<Self_eval> self_eval : model) {
        auto row = table->rowAt(table->rowCount());

        auto eval_url = self_eval->eval_url();
        auto grade_url = self_eval->grade_url();
        auto grader = self_eval->grader_eval()->grader()->nice_name(false);

        row->elementAt(0)->addNew<Wt::WAnchor>(
                Wt::WLink(Wt::LinkType::InternalPath, eval_url),
                eval_url);
        row->elementAt(1)->addNew<Wt::WText>(grader);
        row->elementAt(2)->addNew<Wt::WAnchor>(
                Wt::WLink(Wt::LinkType::InternalPath, grade_url),
                grade_url);
    }
}

Notable_self_evals_view::Notable_self_evals_view(Session& session)
        : session_(session)
{
    auto container = setNewImplementation<Wt::WContainerWidget>();
    setStyleClass("held-back");

    dbo::Transaction transaction(session_);

    if (auto regrade = Self_eval::find_with_grade_status(
                Grader_eval::Status::regrade, session_);
            !regrade.empty()) {
        container->addNew<Wt::WText>("<h3>Regrade</h3>");
        container->addNew<Self_eval_table>(regrade);
    }

    if (auto held_back = Self_eval::find_with_grade_status(
                Grader_eval::Status::held_back, session_);
            !held_back.empty()) {
        container->addNew<Wt::WText>("<h3>Held back</h3>");
        container->addNew<Self_eval_table>(held_back);
    }

    if (auto editing = Self_eval::find_with_grade_status(
                Grader_eval::Status::editing, session_);
            !editing.empty()) {
        container->addNew<Wt::WText>("<h3>Editing</h3>");
        container->addNew<Self_eval_table>(editing);
    }
}

#include "Held_back_view.h"
#include "../Navigate.h"
#include "../model/Assignment.h"
#include "../model/Eval_item.h"
#include "../model/Self_eval.h"
#include "../model/Session.h"

#include <Wt/WAnchor.h>
#include <Wt/WLink.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

class Self_eval_table : public Wt::WContainerWidget
{
public:
    Self_eval_table(const dbo::collection<dbo::ptr<Self_eval>>&,
                    Wt::WContainerWidget* parent = nullptr);
};

Self_eval_table::Self_eval_table(
        const dbo::collection<dbo::ptr<Self_eval>>& model,
        Wt::WContainerWidget* parent)
        : WContainerWidget(parent)
{
    auto table = new Wt::WTable(this);

    for (dbo::ptr<Self_eval> self_eval : model) {
        auto row = table->rowAt(table->rowCount());

        auto eval_url = self_eval->eval_url();
        auto grade_url = self_eval->grade_url();
        auto grader = self_eval->grader_eval()->grader()->name();

        new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, eval_url),
                        eval_url,
                        row->elementAt(0));
        new Wt::WText(grader, row->elementAt(1));
        new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, grade_url),
                        grade_url,
                        row->elementAt(2));
    }
}

Held_back_view::Held_back_view(Session& session, Wt::WContainerWidget* parent)
        : WCompositeWidget(parent),
          session_(session)
{
    auto container = new Wt::WContainerWidget;
    setImplementation(container);
    setStyleClass("held-back");

    dbo::Transaction transaction(session_);

    {
        auto held_back = Self_eval::find_with_grade_status(
                Grader_eval::Status::held_back, session_);
        if (!held_back.empty()) {
            new Wt::WText("<h4>Held back</h4>", container);
            new Self_eval_table(held_back, container);
        }
    }

    {
        auto editing = Self_eval::find_with_grade_status(
                Grader_eval::Status::editing, session_);
        if (!editing.empty()) {
            new Wt::WText("<h4>Editing</h4>", container);
            new Self_eval_table(editing, container);
        }
    }
}

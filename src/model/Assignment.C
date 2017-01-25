#include "Assignment.h"
#include "Eval_item.h"
#include "File_data.h"
#include "Grader_eval.h"
#include "Submission.h"
#include "auth/User.h"
#include "Self_eval.h"
#include "File_meta.h"

#include <Wt/Dbo/Impl>

DBO_INSTANTIATE_TEMPLATES(Assignment);

Assignment::Assignment(int number,
                       const std::string& name,
                       int points,
                       const Wt::WDateTime& open_date,
                       const Wt::WDateTime& due_date,
                       const Wt::WDateTime& eval_date)
        : number_(number), name_(name), points_(points),
          open_date_(open_date), due_date_(due_date), eval_date_(eval_date)
{

}

Assignment::Status Assignment::status() const
{
    auto now = Wt::WDateTime::currentDateTime();

    if (now < open_date_)
        return Status::Future;
    else if (now <= eval_date_)
        return Status::Self_eval;
    else if (now <= due_date_)
        return Status::Open;
    else return Status::Closed;
}

dbo::ptr<Assignment>
Assignment::find_by_number(dbo::Session& dbo, int number)
{
    return dbo.find<Assignment>()
              .where("number = ?")
              .bind(number);
}

Eval_items Assignment::eval_items() const
{
    return eval_items_.session()->find<Eval_item>()
                      .where("assignment_number = ?").bind(number())
                      .orderBy("sequence");
}

double Assignment::total_relative_value() const
{
    return eval_items_.session()->query<double>(
            "SELECT SUM(relative_value)"
            "  FROM eval_items"
            " WHERE assignment_number = ?"
    ).bind(number()).resultValue();
}


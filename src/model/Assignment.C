#include "Assignment.h"
#include "Eval_item.h"
#include "File_data.h"
#include "File_meta.h"
#include "Grader_eval.h"
#include "Self_eval.h"
#include "Submission.h"
#include "auth/User.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Dbo/Transaction.h>

#include <iomanip>
#include <sstream>

DBO_INSTANTIATE_TEMPLATES(Assignment)

Assignment::Assignment(int number, const std::string &name, int points,
                       const Wt::WDateTime &open_date,
                       const Wt::WDateTime &due_date,
                       const Wt::WDateTime &eval_date)
    : number_(number), name_(name), points_(points), open_date_(open_date),
      due_date_(due_date), eval_date_(eval_date) {}

Assignment::Status Assignment::status() const {
  auto now = Wt::WDateTime::currentDateTime();

  if (now < open_date_)
    return Status::Future;
  else if (now <= eval_date_)
    return Status::Self_eval;
  else if (now <= due_date_)
    return Status::Open;
  else
    return Status::Closed;
}

std::string Assignment::slug_string() const {
  std::ostringstream os;
  os << "hw" << std::setfill('0') << std::setw(2) << number();
  return os.str();
}

dbo::ptr<Assignment> Assignment::find_by_number(dbo::Session &dbo, int number) {
  return dbo.find<Assignment>().where("number = ?").bind(number);
}

Eval_items Assignment::eval_items() const {
  return eval_items_.session()
      ->find<Eval_item>()
      .where("assignment_number = ?")
      .bind(number())
      .orderBy("sequence");
}

Eval_item_vec Assignment::eval_item_vec() const {
  auto query = eval_items();
  return Eval_item_vec(query.begin(), query.end());
}

double Assignment::total_relative_value() const {
  return eval_items_.session()
      ->query<double>("SELECT SUM(relative_value)"
                      "  FROM eval_item"
                      " WHERE assignment_number = ?")
      .bind(number())
      .resultValue();
}

dbo::ptr<Eval_item> Assignment::find_eval_item(dbo::Session &session,
                                               int sequence) const {
  return session.find<Eval_item>()
      .where("assignment_number = ?")
      .bind(number())
      .where("sequence = ?")
      .bind(sequence);
}

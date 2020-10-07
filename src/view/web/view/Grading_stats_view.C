#include "../../../Navigate.h"
#include "../../../Session.h"
#include "../../../model/auth/User.h"
#include "../widget/Accelerator_button.h"
#include "Grading_stats_view.h"

#include <Wt/WTable.h>
#include <Wt/WText.h>

#include <map>
#include <set>
#include <string>
#include <tuple>

struct Grader_stats_view_model_row {
  dbo::ptr<User> user;
  std::map<int, int> counts;
  int total_count;
};

using Grader_stats_view_model =
    std::map<std::string, Grader_stats_view_model_row>;

Grading_stats_view::Grading_stats_view(Session &session) : session_(session) {
  Grader_stats_view_model model;

  dbo::Transaction transaction(session_);
  //    for (auto user : session.find<User>()
  //                            .where("role <> ?")
  //                            .bind((int) User::Role::Student)
  //                            .resultList()) {
  //        model[user->name()] = {user, {}, 0};
  //    }

  static const std::string query =
      "SELECT u.name, e.assignment_number, COUNT(*)"
      " FROM gsc_user u"
      " INNER JOIN grader_eval g ON g.grader_id = u.id"
      " INNER JOIN self_eval s ON g.self_eval_id = s.id"
      " INNER JOIN eval_item e ON s.eval_item_id = e.id"
      " WHERE u.role <> ?"
      " GROUP BY u.name, e.assignment_number";
  auto results = session.query<std::tuple<std::string, int, int>>(query)
                     .bind(int(User::Role::Student))
                     .resultList();
  for (auto row : results) {
    const std::string &user_name = std::get<0>(row);
    int assignment_number = std::get<1>(row);
    int count = std::get<2>(row);

    auto &row_model = model[user_name];
    row_model.counts[assignment_number] = count;
    row_model.total_count += count;
  }
  transaction.commit();

  std::set<int> assignments;

  auto impl = setNewImplementation<Wt::WContainerWidget>();
  setStyleClass("grading-stats-view");

  auto table = impl->addNew<Wt::WTable>();
  table->setHeaderCount(1, Wt::Orientation::Horizontal);
  table->setHeaderCount(1, Wt::Orientation::Vertical);

  int row = 1;
  for (const auto &p : model) {
    table->elementAt(row, 0)->addNew<Wt::WText>(p.first);
    table->elementAt(row, 1)->addNew<Wt::WText>(
        std::to_string(p.second.total_count));

    for (const auto &q : p.second.counts) {
      table->elementAt(row, q.first + 1)
          ->addNew<Wt::WText>(std::to_string(q.second));
      assignments.insert(q.first);
    }

    ++row;
  }

  table->elementAt(0, 1)->addNew<Wt::WText>("Total");
  for (int asst : assignments) {
    table->elementAt(0, asst + 1)
        ->addNew<Wt::WText>("hw" + std::to_string(asst));
  }

  auto buttons = impl->addNew<Wt::WContainerWidget>();
  buttons->setStyleClass("buttons");
  auto grade_button = buttons->addNew<Accelerator_button>("&Grade");
  grade_button->clicked().connect(Navigate("/grade"));
}

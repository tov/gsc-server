#include "Grading_stats_view.h"
#include "Accelerator_button.h"
#include "../Navigate.h"
#include "../model/auth/User.h"
#include "../model/Session.h"

#include <Wt/WTable>
#include <Wt/WText>

#include <map>
#include <set>
#include <string>
#include <tuple>

struct Grader_stats_view_model_row {
    dbo::ptr<User> user;
    std::map<int, int> counts;
    int total_count;
};

using Grader_stats_view_model
    = std::map<std::string, Grader_stats_view_model_row>;

Grading_stats_view::Grading_stats_view(Session& session,
                                     Wt::WContainerWidget* parent)
        : WCompositeWidget(parent),
          session_(session)
{
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
                    " FROM users u"
                    " INNER JOIN grader_evals g ON g.grader_id = u.id"
                    " INNER JOIN self_evals s ON g.self_eval_id = s.id"
                    " INNER JOIN eval_items e ON s.eval_item_id = e.id"
                    " GROUP BY u.name, e.assignment_number";
    for (auto row : session.query<boost::tuple<std::string, int, int>>
                                   (query).resultList()) {
        const std::string& user_name = boost::get<0>(row);
        int assignment_number = boost::get<1>(row);
        int count = boost::get<2>(row);

        auto& row_model = model[user_name];
        row_model.counts[assignment_number] = count;
        row_model.total_count += count;
    }
    transaction.commit();

    std::set<int> assignments;

    auto impl = new Wt::WContainerWidget;
    setImplementation(impl);
    setStyleClass("grading-stats-view");

    auto table = new Wt::WTable(impl);
    table->setHeaderCount(1, Wt::Horizontal);
    table->setHeaderCount(1, Wt::Vertical);

    int row = 1;
    for (const auto& p : model) {
        new Wt::WText(p.first, table->elementAt(row, 0));
        new Wt::WText(boost::lexical_cast<std::string>(p.second.total_count),
                      table->elementAt(row, 1));

        for (const auto& q : p.second.counts) {
            new Wt::WText(boost::lexical_cast<std::string>(q.second),
                          table->elementAt(row, q.first + 1));
            assignments.insert(q.first);
        }

        ++row;
    }

    new Wt::WText("Total", table->elementAt(0, 1));
    for (int asst : assignments) {
        new Wt::WText("hw" + boost::lexical_cast<std::string>(asst),
                      table->elementAt(0, asst + 1));
    }

    auto buttons = new Wt::WContainerWidget(impl);
    buttons->setStyleClass("buttons");
    auto grade_button = new Accelerator_button("&Grade", buttons);
    grade_button->clicked().connect(Navigate("/grade"));
}

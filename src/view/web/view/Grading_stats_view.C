#include "Grading_stats_view.h"
#include "../widget/Accelerator_button.h"
#include "../../../Navigate.h"
#include "../../../model/auth/User.h"
#include "../../../Session.h"
#include "../../../common/util.h"

#include <Wt/WTable.h>
#include <Wt/WText.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace {

struct Grader_stats {
    dbo::ptr<User>  user;
    map<int, int>   count_by_asst;
    int             total_count = 0;

    void record(
            dbo::Session& session,
            string const& user_name,
            int assignment_number,
            int count);
};

void Grader_stats::record(
        dbo::Session& session,
        string const& user_name,
        int assignment_number,
        int count)
{
    if (!user) user = User::find_by_name(session, user_name);
    count_by_asst[assignment_number] = count;
    total_count += count;
}

using View_model = vector<Grader_stats>;

string const grading_stats_query
    = "SELECT u.name, e.assignment_number, COUNT(*)"
      " FROM       gsc_user    u"
      " INNER JOIN grader_eval g ON u.id = g.grader_id"
      " INNER JOIN self_eval   s ON s.id = g.self_eval_id"
      " INNER JOIN eval_item   e ON e.id = s.eval_item_id"
      " WHERE u.role <> ?"
      " GROUP BY u.name, e.assignment_number";

bool
operator<(Grader_stats const& a, Grader_stats const& b)
{
    return a.total_count > b.total_count;
}

View_model
load_grading_stats(Session& session)
{
    // First load everything from the database:

    dbo::Transaction transaction(session);
    map<string, Grader_stats> grader_map;

    using result_tuple = tuple<string, int, int>;
    auto rows = session.query<result_tuple>(grading_stats_query)
                          .bind(int(User::Role::Student))
                          .resultList();

    for (auto row : rows) {
        auto const& name = get<0>(row);
        int number       = get<1>(row);
        int count        = get<2>(row);
        grader_map[name].record(session, name, number, count);
    }

    // Then copy to a vector, sort, and return:

    View_model result;
    result.reserve(grader_map.size());

    for (auto& [_, grader_stats] : grader_map) {
        result.push_back(move(grader_stats));
    }

    sort(begin(result), end(result));

    return result;
}

set<int>
build_assignment_set(View_model const& model)
{
    set<int> result;

    for (auto const& stats : model) {
        for (auto [asst, _] : stats.count_by_asst) {
            result.insert(asst);
        }
    }

    return result;
}

void
add_count(WTableRow* row, int col_num, int count)
{
    auto cell = row->elementAt(col_num);
    cell->addNew<WText>(to_string(count));
    cell->setStyleClass("numeric");
}

}  // end anonymous namespace


Grading_stats_view::Grading_stats_view(Session& session)
        : session_(session)
{
    auto all_stats    = load_grading_stats(session_);
    auto asst_set     = build_assignment_set(all_stats);
    int total_col_num = asst_set.size() + 1;

    auto impl = setNewImplementation<WContainerWidget>();
    setStyleClass("grading-stats-view");

    auto table = impl->addNew<WTable>();
    table->setHeaderCount(1, Orientation::Horizontal);
    table->setHeaderCount(1, Orientation::Vertical);

    auto top_row = table->rowAt(0);
    top_row->elementAt(total_col_num)->addNew<WText>("Total");
    for (int asst : asst_set) {
        top_row->elementAt(asst)->addNew<WText>("hw" + to_string(asst));
    }

    for (auto const& grader : all_stats) {
        auto row = table->rowAt(table->rowCount());

        row->elementAt(0)->addNew<WText>(grader.user->html_name(false));

        for (auto [asst, count] : grader.count_by_asst) {
            add_count(row, asst, count);
        }

        add_count(row, total_col_num, grader.total_count);
    }

    auto buttons = impl->addNew<WContainerWidget>();
    buttons->setStyleClass("buttons");

    auto grade_button = buttons->addNew<Accelerator_button>("&Grade");
    grade_button->clicked().connect(Navigate("/grade"));
}

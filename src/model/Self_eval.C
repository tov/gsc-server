#include "Self_eval.h"
#include "Assignment.h"
#include "Eval_item.h"
#include "Submission.h"
#include "Grader_eval.h"
#include "../common/paths.h"
#include "../common/format.h"
#include "../common/util.h"

#include "Permalink.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Value.h>
#include <Wt/WDateTime.h>

#include <algorithm>

namespace J = Wt::Json;

using namespace std;

DBO_INSTANTIATE_TEMPLATES(Self_eval)

static double default_score(Eval_item::Type type)
{
    if (type == Eval_item::Type::Boolean)
        return 0;
    else
        return 1;
}

dbo::ptr<Self_eval>
Self_eval::create(const dbo::ptr<Eval_item>& eval_item,
                  const dbo::ptr<Submission>& submission)
{
    auto& session = *eval_item.session();

    auto result = session.add(unique_ptr<Self_eval>{
            new Self_eval{eval_item, submission}});

    if (eval_item->is_graded_automatically()) {
        session.addNew<Grader_eval>(result, submission->user1(), 1);
    }

    submission.modify()->touch();

    return result;
}

Self_eval::Self_eval(const dbo::ptr<Eval_item>& eval_item,
                     const dbo::ptr<Submission>& submission)
        : Abstract_evaluation(default_score(eval_item->type())),
          eval_item_(eval_item),
          submission_(submission),
          permalink_(create_permalink(permalink_size))
{ }

Self_eval::Freeze_status
Self_eval::get_freeze_status() const
{
    if (eval_item()->is_informational())
        return Freeze_status::full;

    if (auto graded = grader_eval(); graded) {
        if (graded->score() == 1)
            return Freeze_status::full;

        if (eval_item()->type() == Eval_item::Type::Boolean
            && score() == 1)
            return Freeze_status::score;
    }

    return Freeze_status::none;
}

bool Self_eval::fully_frozen() const
{ return get_freeze_status() == Freeze_status::full; }

bool Self_eval::score_frozen() const
{ return get_freeze_status() == Freeze_status::score; }

std::string Self_eval::eval_url() const
{
    std::ostringstream fmt;

    fmt << "/~" << submission()->user1()->name();
    fmt << "/hw/" << submission()->assignment()->number();
    fmt << "/eval/" << eval_item()->sequence();

    return fmt.str();
}

std::string Self_eval::grade_url() const
{
    return "/grade/" + permalink();
}

dbo::ptr<Self_eval>
Self_eval::find_by_permalink(dbo::Session& dbo,
                             std::string const& permalink)
{
    return dbo.find<Self_eval>()
              .where("permalink = ?").bind(permalink);
}

namespace {

// If you've been editing for over an hour, forget about it.
void drop_old_editing_holds(dbo::Session& dbo)
{
    dbo.execute("DELETE FROM grader_eval"
                " WHERE status = ?"
                "   AND time_stamp < NOW() AT TIME ZONE 'UTC' - INTERVAL '1 hr'")
            .bind((int) Grader_eval::Status::editing);
}

#define ELIGIBLE_FROM_AND_WHERE_CLAUSE \
    " FROM self_eval" \
    " INNER JOIN submission" \
    "    ON submission.id = self_eval.submission_id" \
    " INNER JOIN eval_item" \
    "   ON eval_item.id = self_eval.eval_item_id" \
    " INNER JOIN assignment" \
    "   ON assignment.number = eval_item.assignment_number" \
    " LEFT OUTER JOIN grader_eval" \
    "   ON grader_eval.self_eval_id = self_eval.id" \
    " WHERE grader_eval.self_eval_id IS NULL" \
    "   AND assignment.due_date < utc_now()" \
    "   AND submission.due_date < utc_now()"

// Returns the permalink ID for the user's current editing hold, or
// the empty string if none.
string
current_editing_hold(dbo::Session& dbo,
                     dbo::ptr<User> const& user)
{
    return dbo.query<std::string>(
                      "SELECT s.permalink"
                      "  FROM self_eval s"
                      " INNER JOIN grader_eval g ON g.self_eval_id = s.id"
                      " WHERE g.grader_id = ?"
                      "   AND g.status = ?"
                      " LIMIT 1"
              )
              .bind(user.id())
              .bind((int) Grader_eval::Status::editing)
              .resultValue();
}

// Returns the permalink for the next self eval eligible for grading, or
// the empty string if none.
string
next_eligible_permalink(dbo::Session& dbo)
{
    return dbo.query<string>(
            "SELECT self_eval.permalink"
            ELIGIBLE_FROM_AND_WHERE_CLAUSE
            " ORDER BY"
            "   self_eval.time_stamp,"
            "   assignment.number,"
            "   eval_item.sequence"
            " LIMIT 1")
              .resultValue();
}

// Returns how many self evaluations are eligible for grading.
int count_eligible(dbo::Session& dbo)
{
    return dbo.query<int>("SELECT COUNT(*)"
                          ELIGIBLE_FROM_AND_WHERE_CLAUSE);
}

}

string
Self_eval::find_ungraded_permalink(dbo::Session& dbo,
                                   const dbo::ptr<User>& user)
{
    drop_old_editing_holds(dbo);

    if (auto current = current_editing_hold(dbo, user);
            !current.empty())
        return current;

    return next_eligible_permalink(dbo);
}

dbo::collection<dbo::ptr<Self_eval>>
Self_eval::find_with_grade_status(Grader_eval::Status status,
                                  dbo::Session& dbo)
{
    return dbo.query<dbo::ptr<Self_eval>>(
            "SELECT s"
            " FROM self_eval s"
            " INNER JOIN eval_item e ON s.eval_item_id = e.id"
            " INNER JOIN assignment a ON e.assignment_number = a.number"
            " INNER JOIN grader_eval g ON s.id = g.self_eval_id"
            " WHERE g.status = ?"
            " ORDER BY a.number, e.sequence"
    ).bind((int) status).resultList();
}

std::string Self_eval::rest_uri() const {
    return api::paths::Submissions_1_evals_2_self(submission()->id(),
                                                  eval_item()->sequence());
}

J::Object Self_eval::to_json(Viewing_context const&) const {
    J::Object result = Abstract_evaluation::to_json();

    result["uri"]               = J::Value(rest_uri());
    result["permalink"]         = J::Value(permalink());

    return result;
}

Score_owner Self_eval::score_owner(Viewing_context const& cxt) const
{
    WString score = cxt.viewer->role() != User::Role::Grader
                   ? plain_score_string()
                   : "[***]";
    WString owner = submission()->owner_string(cxt);
    return {score, owner};
}

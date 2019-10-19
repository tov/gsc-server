#include "Self_eval.h"
#include "Assignment.h"
#include "Eval_item.h"
#include "Submission.h"
#include "Grader_eval.h"
#include "../common/paths.h"
#include "../common/format.h"

#include "Permalink.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Value.h>
#include <Wt/WDateTime.h>

#include <algorithm>

namespace J = Wt::Json;

DBO_INSTANTIATE_TEMPLATES(Self_eval)

static double default_score(Eval_item::Type type)
{
    if (type == Eval_item::Type::Boolean)
        return 0;
    else
        return 1;
}

Self_eval::Self_eval(const dbo::ptr<Eval_item>& eval_item,
                     const dbo::ptr<Submission>& submission)
        : Abstract_evaluation(default_score(eval_item->type())),
          eval_item_(eval_item),
          submission_(submission),
          permalink_(create_permalink(permalink_size))
{ }

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
                             const std::string& permalink)
{
    return dbo.find<Self_eval>()
              .where("permalink = ?").bind(permalink);
}

std::string Self_eval::find_ungraded_permalink(dbo::Session& dbo,
                                               const dbo::ptr<User>& user)
{
    dbo.execute("DELETE FROM grader_eval"
                " WHERE status = ?"
                "   AND time_stamp < NOW() AT TIME ZONE 'UTC' - INTERVAL '1 hr'")
            .bind((int) Grader_eval::Status::editing);

    std::string current =
            dbo.query<std::string>(
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
    if (!current.empty()) return current;

    return dbo.query<std::string>(
            "SELECT s.permalink"
            " FROM self_eval s"
            " INNER JOIN submission b ON b.id = s.submission_id"
            " INNER JOIN eval_item e ON s.eval_item_id = e.id"
            " INNER JOIN assignment a ON e.assignment_number = a.number"
            " LEFT OUTER JOIN grader_eval g ON g.self_eval_id = s.id"
            " WHERE g.self_eval_id IS NULL"
            "   AND e.type <> ?"
            "   AND a.eval_date < NOW() AT TIME ZONE 'UTC'"
            "   AND b.eval_date < NOW() AT TIME ZONE 'UTC'"
            " ORDER BY a.number, e.sequence"
            " LIMIT 1"
    ).bind(static_cast<int>(Eval_item::Type::Informational))
            .resultValue();
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

std::string Self_eval::owner_string(const dbo::ptr<User>& as_seen_by) const
{
    if (as_seen_by == submission()->user1() ||
            as_seen_by == submission()->user2())
        return "You";

    switch (as_seen_by->role()) {
        case User::Role::Student:
            return "Other student";
        case User::Role::Grader:
            return "Student";
        case User::Role::Admin:
            return submission()->owner_string();
    }
}

std::string Self_eval::rest_uri() const {
    return api::paths::Submissions_1_evals_2_self(submission()->id(),
                                                  eval_item()->sequence());
}

J::Object Self_eval::to_json() const {
    J::Object result = Abstract_evaluation::to_json();

    result["uri"]               = J::Value(rest_uri());
    result["permalink"]         = J::Value(permalink());

    return result;
}


#include "Self_eval.h"
#include "Assignment.h"
#include "Eval_item.h"
#include "Submission.h"
#include "Grader_eval.h"
#include "../common/paths.h"

#include "Permalink.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Value.h>
#include <Wt/WDateTime.h>

namespace J = Wt::Json;

DBO_INSTANTIATE_TEMPLATES(Self_eval)

Self_eval::Self_eval(const dbo::ptr<Eval_item>& eval_item,
                     const dbo::ptr<Submission>& submission)
        : eval_item_(eval_item),
          submission_(submission),
          time_stamp_(Wt::WDateTime::currentDateTime()),
          permalink_(create_permalink(permalink_size))
{

}

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

void Self_eval::touch_()
{
    time_stamp_ = Wt::WDateTime::currentDateTime();
}

void Self_eval::set_explanation(const std::string& explanation)
{
    explanation_ = explanation;
    touch_();
}

void Self_eval::set_score(double score)
{
    score_ = score;
    touch_();
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
    dbo.execute("DELETE FROM grader_evals"
                " WHERE status = ?"
                "   AND time_stamp < NOW() AT TIME ZONE 'UTC' - INTERVAL '1 hr'")
            .bind((int) Grader_eval::Status::editing);

    std::string current =
            dbo.query<std::string>(
                    "SELECT s.permalink"
                    "  FROM self_evals s"
                    " INNER JOIN grader_evals g ON g.self_eval_id = s.id"
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
            " FROM self_evals s"
            " INNER JOIN submissions b ON b.id = s.submission_id"
            " INNER JOIN eval_items e ON s.eval_item_id = e.id"
            " INNER JOIN assignments a ON e.assignment_number = a.number"
            " LEFT OUTER JOIN grader_evals g ON g.self_eval_id = s.id"
            " WHERE g.self_eval_id IS NULL"
            "   AND a.eval_date < NOW() AT TIME ZONE 'UTC'"
            "   AND b.eval_date < NOW() AT TIME ZONE 'UTC'"
            " ORDER BY a.number, e.sequence"
            " LIMIT 1"
    ).resultValue();
}

dbo::collection<dbo::ptr<Self_eval>>
Self_eval::find_with_grade_status(Grader_eval::Status status,
                                  dbo::Session& dbo)
{
    return dbo.query<dbo::ptr<Self_eval>>(
            "SELECT s"
            " FROM self_evals s"
            " INNER JOIN eval_items e ON s.eval_item_id = e.id"
            " INNER JOIN assignments a ON e.assignment_number = a.number"
            " INNER JOIN grader_evals g ON s.id = g.self_eval_id"
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

static J::Value clean_grade(double grade) {
    return grade < 0.001? 0 : grade;
}

std::string Self_eval::rest_uri() const
{
    return api::paths::Submissions_1_evals_2(submission().id(), eval_item()->sequence());
}

J::Object Self_eval::to_json(bool brief, dbo::ptr<User> const& as_seen_by) const {
    J::Object result;

    result["uri"]               = J::Value(rest_uri());
    result["sequence"]          = J::Value(eval_item()->sequence());
    result["submission_uri"]    = J::Value(submission()->rest_uri());

    if (!brief) {
        result["type"]              = J::Value(stringify(eval_item()->type()));
        result["prompt"]            = J::Value(eval_item()->prompt());
        result["value"]             = J::Value(eval_item()->relative_value());

        result["self_score"]        = clean_grade(score());
        result["self_explanation"]  = J::Value(explanation());

        auto grader_eval = this->grader_eval();
        if (grader_eval) {
            if (grader_eval->status() == Grader_eval::Status::ready ||
                    as_seen_by->can_grade()) {
                result["grader"]             = J::Value(grader_eval->owner_string(as_seen_by));
                result["grader_score"]       = clean_grade(grader_eval->score());
                result["grader_explanation"] = J::Value(grader_eval->explanation());
                result["grading_status"]     = J::Value(stringify(grader_eval->status()));
            }
        }
    }

    return result;
}


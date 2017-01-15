#include "Self_eval.h"
#include "Assignment.h"
#include "Eval_item.h"
#include "Submission.h"
#include "Grader_eval.h"

#include "Permalink.h"

#include <Wt/Dbo/Impl>
#include <Wt/WDateTime>

DBO_INSTANTIATE_TEMPLATES(Self_eval);

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

std::string Self_eval::find_ungraded_permalink(dbo::Session& dbo)
{
    dbo.execute("DELETE FROM grader_evals"
                " WHERE status = ?"
                "   AND time_stamp < NOW() AT TIME ZONE 'UTC' - INTERVAL '1 hr'")
            .bind((int) Grader_eval::Status::editing);

    return dbo.query<std::string>(
            "SELECT s.permalink"
            " FROM self_evals s"
            " INNER JOIN eval_items e ON s.eval_item_id = e.id"
            " INNER JOIN assignments a ON e.assignment_number = a.number"
            " LEFT OUTER JOIN grader_evals g ON s.id = g.self_eval_id"
            " WHERE g.self_eval_id IS NULL"
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



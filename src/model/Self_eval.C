#include "Self_eval.h"
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

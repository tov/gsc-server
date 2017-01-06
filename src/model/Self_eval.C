#include "Self_eval.h"
#include "Eval_item.h"
#include "Submission.h"
#include "Grader_eval.h"

#include <Wt/Dbo/Impl>
#include <Wt/WDateTime>

DBO_INSTANTIATE_TEMPLATES(Self_eval);

Self_eval::Self_eval(const dbo::ptr<Eval_item>& eval_item,
                     const dbo::ptr<Submission>& submission)
        : eval_item_(eval_item),
          submission_(submission),
          time_stamp_(Wt::WDateTime::currentDateTime())
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

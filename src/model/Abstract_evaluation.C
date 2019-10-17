#include "Abstract_evaluation.h"
#include "Eval_item.h"
#include "Submission.h"

#include <type_traits>

void Abstract_evaluation::set_explanation(const std::string& explanation)
{
    explanation_ = explanation;
}

void Abstract_evaluation::set_score(double score)
{
    if (score < std::numeric_limits<double>::min()) {
        score_ = 0;
    } else if (score > 1) {
        score_ = 1;
    } else {
        score_ = score;
    }
}

const dbo::ptr<Assignment>& Abstract_evaluation::assignment() const
{
    return submission()->assignment();
}

std::string Abstract_evaluation::score_string() const
{
    return eval_item()->format_score(score());
}

void Abstract_evaluation::touch_()
{
    time_stamp_ = Wt::WDateTime::currentDateTime();
}



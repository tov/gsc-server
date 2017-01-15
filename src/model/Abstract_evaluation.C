#include "Abstract_evaluation.h"

#include "Eval_item.h"
#include "Submission.h"

const dbo::ptr<Assignment>& Abstract_evaluation::assignment() const
{
    return submission()->assignment();
}

std::string Abstract_evaluation::score_string() const
{
    return eval_item()->format_score(score());
}

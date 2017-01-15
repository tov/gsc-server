#include "Abstract_evaluation.h"

#include "Submission.h"

const dbo::ptr<Assignment>& Abstract_evaluation::assignment() const
{
    return submission()->assignment();
}

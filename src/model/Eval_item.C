#include "Eval_item.h"
#include "Assignment.h"
#include "Grader_eval.h"
#include "Self_eval.h"
#include "Submission.h"

#include <Wt/Dbo/Impl>

#include <cstdlib>
#include <iomanip>
#include <sstream>

DBO_INSTANTIATE_TEMPLATES(Eval_item);

Eval_item::Eval_item(const dbo::ptr<Assignment>& assignment, int sequence)
        : assignment_(assignment), sequence_(sequence)
{

}

std::string Eval_item::relative_value_str() const
{
    std::ostringstream os;
    os << std::setprecision(3) << relative_value_;
    return os.str();
}

void Eval_item::set_relative_value(const std::string& s)
{
    set_relative_value(atof(s.data()));
}

std::string Eval_item::format_score(double score) const
{
    if (type() == Type::Boolean) {
        if (score == 0) return "No";
        if (score == 1) return "Yes";
    }

    if (type() == Type::Informational && relative_value() == 0)
        return "Okay";

    return pct_string(score);
}

std::string Eval_item::pct_string(double ratio)
{
    if (ratio == 1.0) return "100%";

    std::ostringstream fmt;
    fmt << std::setprecision(2) << 100 * ratio << '%';
    return fmt.str();
}

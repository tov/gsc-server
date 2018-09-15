#include "Eval_item.h"
#include "Assignment.h"
#include "Grader_eval.h"
#include "Self_eval.h"
#include "Submission.h"

#include <Wt/Dbo/Impl.h>

#include <cstdlib>
#include <iomanip>
#include <sstream>

DBO_INSTANTIATE_TEMPLATES(Eval_item)

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

std::string Eval_item::pct_string(double ratio, int precision)
{
    if (ratio == 1.0) return "100%";
    if (ratio < 0.0001) return "0%";

    std::ostringstream fmt;
    fmt << std::setprecision(precision) << 100 * ratio << '%';
    return fmt.str();
}

double Eval_item::absolute_value() const
{
    double denominator = assignment()->total_relative_value();
    return denominator == 0 ? 0 : relative_value() / denominator;
}

std::string Eval_item::absolute_value_str() const
{
    return pct_string(absolute_value(), 3);
}

std::ostream& operator<<(std::ostream& o, Eval_item::Type type)
{
    switch (type) {
        case Eval_item::Type::Boolean:
            return o << "Boolean";
        case Eval_item::Type::Scale:
            return o << "Scale";
        case Eval_item::Type::Informational:
            return o << "Informational";
    }
}

char const *Enum<Eval_item::Type>::show(Eval_item::Type type) {
    using T = Eval_item::Type;

    switch (type) {
        case T::Boolean:       return "boolean";
        case T::Scale:         return "scale";
        case T::Informational: return "informational";
    }
}

namespace rc = std::regex_constants;

static std::regex const boolean_re("boolean", rc::icase);
static std::regex const scale_re("scale", rc::icase);
static std::regex const informational_re("informational", rc::icase);

Eval_item::Type Enum<Eval_item::Type>::read(char const* type) {
    using T = Eval_item::Type;

    if (std::regex_match(type, boolean_re))
        return T::Boolean;

    if (std::regex_match(type, scale_re))
        return T::Scale;

    if (std::regex_match(type, informational_re))
        return T::Informational;

    throw std::invalid_argument{"Could not parse eval item type"};
}

#include "Grader_eval.h"
#include "Self_eval.h"
#include "Eval_item.h"
#include "Submission.h"
#include "auth/User.h"
#include "../Session.h"
#include "../common/format.h"
#include "../common/stringify.h"
#include "../common/paths.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Json/Value.h>

#include <algorithm>

namespace J = Wt::Json;

DBO_INSTANTIATE_TEMPLATES(Grader_eval)

Grader_eval::Grader_eval(const dbo::ptr<Self_eval>& self_eval,
                         const dbo::ptr<User>& grader)
        : self_eval_(self_eval),
          grader_(grader),
          status_(static_cast<int>(Status::editing))
{ }

Grader_eval::Grader_eval(const dbo::ptr<Self_eval>& self_eval,
                         const dbo::ptr<User>& grader,
                         double score,
                         const std::string& explanation)
        : Abstract_evaluation(score, explanation),
          self_eval_(self_eval),
          grader_(grader),
          status_(static_cast<int>(Status::ready))
{ }

dbo::ptr<Grader_eval> Grader_eval::get_for(
        const dbo::ptr<Self_eval>& self_eval,
        Session& session)
{
    auto result = self_eval->grader_eval();

    if (result)
        return result;
    else
        return session.addNew<Grader_eval>(self_eval, session.user());
}

std::string Grader_eval::plain_score_string() const
{
    switch (status()) {
        case Status::ready:
            return Abstract_evaluation::plain_score_string();
        case Status::held_back:
            return "[held back]";
        case Status::editing:
            return "[editing]";
    }
}

std::string Grader_eval::score_string(Viewing_context const& cxt) const
{
    if (can_see_score(cxt))
        return plain_score_string();
    else
        return "";
}

std::string Grader_eval::owner_string(Viewing_context const& cxt) const
{
    if (! grader()->can_grade())
        return "Auto";

    if (! cxt.viewer->can_grade())
        return "Grader";

    return grader()->name();
}

bool Grader_eval::can_see_score(Viewing_context const& cxt) const
{
    auto grading_status = submission()->grading_status();

    return grading_status == Submission::Grading_status::complete
           || cxt.viewer->can_grade()
           || ! grader()->can_grade()
           || eval_item()->is_informational()
           || (grading_status == Submission::Grading_status::regrade &&
               status() == Status::ready);
}

const Wt::Dbo::ptr<Eval_item>& Grader_eval::eval_item() const
{
    return self_eval()->eval_item();
}

const Wt::Dbo::ptr<Submission>& Grader_eval::submission() const
{
    return self_eval()->submission();
}

bool Grader_eval::can_view(dbo::ptr<User> const& user) const {
    return user->can_grade() ||
            (self_eval()->submission()->can_view(user) && status() == Status::ready);
}


std::string Grader_eval::rest_uri() const {
    return api::paths::Submissions_1_evals_2_grader(self_eval()->submission()->id(),
                                                    self_eval()->eval_item()->sequence());
}

Wt::Json::Object Grader_eval::to_json(Viewing_context const& cxt) const {
    J::Object result = Abstract_evaluation::to_json();

    result["uri"]           = J::Value(rest_uri());
    result["grader"]        = J::Value(owner_string(cxt));
    result["status"]        = J::Value(stringify(status()));

    return result;
}

namespace rc = std::regex_constants;

static std::regex const editing_re("editing", rc::icase);
static std::regex const held_back_re("held_back", rc::icase);
static std::regex const ready_re("ready", rc::icase);

char const* Enum<Grader_eval::Status>::show(Grader_eval::Status role)
{
    switch (role) {
        case Grader_eval::Status::editing: return "editing";
        case Grader_eval::Status::held_back: return "held_back";
        case Grader_eval::Status::ready: return "ready";
    }
}

Grader_eval::Status Enum<Grader_eval::Status>::read(char const* role)
{
    if (std::regex_match(role, editing_re))
        return Grader_eval::Status::editing;

    if (std::regex_match(role, held_back_re))
        return Grader_eval::Status::held_back;

    if (std::regex_match(role, ready_re))
        return Grader_eval::Status::ready;

    throw std::invalid_argument{"Could not parse role"};
}

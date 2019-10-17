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

std::string Grader_eval::score_string() const
{
    switch (status()) {
        case Status::ready:
            return Abstract_evaluation::score_string();
        case Status::held_back:
            return "[held back]";
        case Status::editing:
            return "[editing]";
    }
}

std::string Grader_eval::owner_string(const dbo::ptr<User>& as_seen_by) const
{
    if (grader()->role() == User::Role::Student && as_seen_by->role() == User::Role::Student)
        return "Auto";

    return grader()->name();
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

Wt::Json::Object Grader_eval::to_json(dbo::ptr<User> const& as_seen_by) const {
    J::Object result = Abstract_evaluation::to_json();

    result["uri"]           = J::Value(rest_uri());
    result["grader"]        = J::Value(owner_string(as_seen_by));
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

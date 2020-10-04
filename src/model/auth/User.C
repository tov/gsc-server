#include "../../common/exceptions.h"
#include "../../common/paths.h"
#include "../Assignment.h"
#include "../Eval_item.h"
#include "../Exam_grade.h"
#include "../File_meta.h"
#include "../Grader_eval.h"
#include "../Partner_request.h"
#include "../Self_eval.h"
#include "../Submission.h"
#include "../game/User_stats.h"
#include "Auth_token.h"
#include "User.h"

#include <Wt/Auth/Identity.h>
#include <Wt/Auth/PasswordHash.h>
#include <Wt/Dbo/Impl.h>

#include <algorithm>
#include <stdexcept>

namespace J = Wt::Json;

DBO_INSTANTIATE_TEMPLATES(User)

User::User(const std::string& name, Role role)
    : name_(name)
    , role_(static_cast<int>(role))
{}

void User::set_call_me(std::string const& s)
{
    call_me_ = s;

    if (call_me_.size() > MAX_CALL_ME_LENGTH)
        call_me_.resize(MAX_CALL_ME_LENGTH);
}

bool User::can_grade() const
{
    return role() == User::Role::Grader || role() == User::Role::Admin;
}

bool User::can_admin() const
{
    return role() == User::Role::Admin;
}

bool User::can_view(const std::string& other_name) const
{
    return can_admin() || name() == other_name;
}

bool User::can_view(const dbo::ptr<User>& other) const
{
    return can_view(other->name());
}

void User::check_can_grade() const
{
    if (! can_grade()) throw Access_check_failed("You aren’t a grader!");
}

void User::check_can_admin() const
{
    if (! can_admin()) throw Access_check_failed("You aren’t an admin!");
}

void User::check_can_view(const std::string& other) const
{
    if (can_admin()) return;

    if (! can_view(other)) throw Access_check_failed("That isn’t yours!");
}

void User::check_can_view(const dbo::ptr<User>& other) const
{
    check_can_view(other->name());
}

dbo::ptr<User> User::find_by_name(dbo::Session& dbo, const std::string& name)
{
    return dbo.find<User>().where("name = ?").bind(name);
}

std::vector<dbo::ptr<Submission>> User::submissions() const
{
    std::vector<dbo::ptr<Submission>> result;

    auto extend = [&](int index) {
        while (result.size() <= index) result.emplace_back();
    };

    auto insert = [&](dbo::ptr<Submission> const& submission) {
        auto index = submission->assignment()->number() - 1;
        extend(index);
        result[index] = submission;
    };

    for (const auto& each : submissions1_) insert(each);
    for (const auto& each : submissions2_) insert(each);

    for (const auto& each : session()->find<Assignment>().resultList()) {
        auto index = each->number() - 1;
        extend(index);
        if (! result[index]) {
            auto new_submission =
                    session()->addNew<Submission>(find_this(), each);
            new_submission.flush();  // Assigns an ID
            insert(new_submission);
        }
    }

    return result;
}

Partner_requests_vec User::outgoing_requests() const
{
    Partner_requests_vec result(outgoing_requests_.begin(),
                                outgoing_requests_.end());
    std::sort(result.begin(), result.end(),
              [](auto const& a, auto const& b) { return *a < *b; });
    return result;
}

Partner_requests_vec User::incoming_requests() const
{
    Partner_requests_vec result(incoming_requests_.begin(),
                                incoming_requests_.end());
    std::sort(result.begin(), result.end(),
              [](auto const& a, auto const& b) { return *a < *b; });
    return result;
}

Exam_grades_vec User::exam_grades() const
{
    Exam_grades_vec result(exam_grades_.begin(), exam_grades_.end());
    std::sort(result.begin(), result.end(), [](auto const& a, auto const& b) {
        return a->number() < b->number();
    });
    return result;
}

std::string User::hw_url() const
{
    return "/~" + name() + "/hw";
}

std::string User::profile_url() const
{
    return "/~" + name() + "/profile";
}

std::string User::rest_uri() const
{
    return api::paths::Users_1(name());
}

std::string User::submissions_rest_uri() const
{
    return api::paths::Users_1_submissions(name());
}

J::Object User::to_json(bool brief) const
{
    J::Object result;

    result["name"] = J::Value(name());
    result["uri"]  = J::Value(rest_uri());

    if (! brief) {
        result["submissions_uri"] = J::Value(submissions_rest_uri());
        result["role"]            = J::Value(role_string());

        // Submissions
        J::Array submissions;
        for (auto const& submission : this->submissions())
            submissions.push_back(submission->to_json(true));
        result["submissions"] = J::Value(std::move(submissions));

        // Partner requests
        J::Array partner_requests;

        for (auto const& outgoing : outgoing_requests()) {
            J::Object request;
            request["assignment_number"] =
                    J::Value(outgoing->assignment()->number());
            request["user"]   = J::Value(outgoing->requestee()->name());
            request["status"] = J::Value("outgoing");
            partner_requests.push_back(std::move(request));
        }

        for (auto const& incoming : incoming_requests()) {
            J::Object request;
            request["assignment_number"] =
                    J::Value(incoming->assignment()->number());
            request["user"]   = J::Value(incoming->requestor()->name());
            request["status"] = J::Value("incoming");
            partner_requests.push_back(std::move(request));
        }

        result["partner_requests"] = J::Value(std::move(partner_requests));

        // Exam grades
        J::Array array;
        for (auto const& exam_grade : exam_grades())
            array.push_back(exam_grade->to_json());
        result["exam_grades"] = J::Value(std::move(array));
    }
    return result;
}

dbo::ptr<User> User::find_this() const
{
    return session()->find<User>().where("id = ?").bind(id());
}

User::Role_info::info_t const User::Role_info::info{
        Enumerator_info{Role::Student, "student"},
        Enumerator_info{Role::Grader, "grader"},
        Enumerator_info{Role::Admin, "admin"},
};

char const* User::Role_info::show(Role role)
{
    return info[to_repr(role)].name;
}

User::Role User::Role_info::read(char const* role)
{
    for (auto const& each : info)
        if (std::strcmp(role, each.name) == 0) return each.value;

    throw std::invalid_argument{"Could not parse role"};
}

int User::Role_info::to_repr(Role role)
{
    return static_cast<int>(role);
}

User::Role User::Role_info::from_repr(int n, Role otherwise)
{
    if (n < N) return static_cast<Role>(n);
    else
        return Role::Student;
}

std::optional<User::Role> User::Role_info::from_repr(int n)
{
    if (n < N) return std::make_optional(static_cast<Role>(n));
    else
        return std::nullopt;
}

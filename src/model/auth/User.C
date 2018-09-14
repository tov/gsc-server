#include "User.h"
#include "Auth_token.h"
#include "../Assignment.h"
#include "../Eval_item.h"
#include "../Exam_grade.h"
#include "../File_data.h"
#include "../File_meta.h"
#include "../Grader_eval.h"
#include "../Partner_request.h"
#include "../Self_eval.h"
#include "../Submission.h"
#include "../game/User_stats.h"

#include <Wt/Auth/PasswordHash.h>
#include <Wt/Dbo/Impl.h>
#include <Wt/Auth/Identity.h>

#include <algorithm>
#include <regex>
#include <stdexcept>

namespace J = Wt::Json;

DBO_INSTANTIATE_TEMPLATES(User)

User::User(const std::string& name)
        : name_(name) {}

bool User::can_grade() const
{
    return role() == User::Role::Grader || role() == User::Role::Admin;
}

bool User::can_admin() const
{
    return role() == User::Role::Admin;
}

dbo::ptr<User> User::find_by_name(dbo::Session& dbo,
                                  const std::string& name)
{
    return dbo.find<User>()
              .where("name = ?")
              .bind(name);
}

Wt::Dbo::ptr<User> User::find_by_auth_token(Wt::Dbo::Session& session,
                                            const std::string& hash)
{
    return session.query<dbo::ptr<User>>(
            "select u from users u join auth_tokens t on u.id = t.user_id")
            .where("t.value = ?").bind(hash)
            .where("t.expires > ?").bind(Wt::WDateTime::currentDateTime());
}

Wt::Auth::PasswordHash User::password() const
{
    return Wt::Auth::PasswordHash(password_method_, password_salt_, password_);
}

void User::set_password(const Wt::Auth::PasswordHash& password)
{
    password_method_ = password.function();
    password_salt_ = password.salt();
    password_ = password.value();
}

int const auth_token_gc_threshold = 50;

void User::add_auth_token(const std::string& value, const Wt::WDateTime& expires)
{
    if (auth_tokens_.size() > auth_token_gc_threshold) {
        auto expired = session()->find<Auth_token>()
                .where("expires < NOW()")
                .resultList();
        for (auto token : expired) token.remove();

        int count = auth_tokens_.size();
        if (count > auth_token_gc_threshold / 2) {
            auto victims = session()->find<Auth_token>()
                    .where("user_id = ?").bind(id())
                    .orderBy("expires")
                    .limit(count - auth_token_gc_threshold / 2)
                    .resultList();
            for (auto token : victims) token.remove();
        }
    }

    auth_tokens_.insert(dbo::ptr<Auth_token>(
            std::make_unique<Auth_token>(value, expires)));
}

void User::remove_auth_token(const std::string& value)
{
    for (auto& token : auth_tokens()) {
        if (token->value == value) {
            token.remove();
            break;
        }
    }
}

int User::update_auth_token(const std::string& old, const std::string& new_)
{
    for (const auto& token : auth_tokens()) {
        if (token->value == old) {
            token.modify()->value = new_;
            return std::max(Wt::WDateTime::currentDateTime()
                                    .secsTo(token->expires),
                            0);
        }
    }

    return 0;
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
        if (!result[index]) {
            auto new_submission = session()->addNew<Submission>(find_this(), each);
            new_submission.flush(); // Assigns an ID
            insert(new_submission);
        }
    }

    return result;
}

Partner_requests User::outgoing_requests() const
{
    return Partner_requests(outgoing_requests_.begin(), outgoing_requests_.end());
}

Partner_requests User::incoming_requests() const
{
    return Partner_requests(incoming_requests_.begin(), incoming_requests_.end());
}

bool User::can_view(const dbo::ptr<User>& other) const
{
    return can_admin() || name() == other->name();
}

std::string User::hw_url() const
{
    return "/~" + name() + "/hw";
}

std::string User::rest_uri() const
{
    std::ostringstream os;
    os << "/api/users/" << name();
    return os.str();
}

std::string User::rest_hw_uri() const
{
    std::ostringstream os;
    os << "/api/users/" << name() << "/hws";
    return os.str();
}

J::Object User::to_json(bool brief) const
{
    J::Object result;
    result["name"] = J::Value(name());
    result["uri"]  = J::Value(rest_uri());
    if (!brief) {
        result["role"] = J::Value(role_string());

        // Submissions
        J::Array submissions;
        for (auto const& submission : this->submissions())
            submissions.push_back(submission->to_json(true));
        result["submissions"] = J::Value(std::move(submissions));

        // Partner requests
        J::Array partner_requests;

        for (auto const& outgoing : outgoing_requests_) {
            J::Object request;
            request["assignment_number"] = J::Value(outgoing->assignment()->number());
            request["user"]              = J::Value(outgoing->requestee()->name());
            request["status"]            = J::Value("outgoing");
            partner_requests.push_back(std::move(request));
        }

        for (auto const& incoming : incoming_requests_) {
            J::Object request;
            request["assignment_number"] = J::Value(incoming->assignment()->number());
            request["user"]              = J::Value(incoming->requestor()->name());
            request["status"]            = J::Value("incoming");
            partner_requests.push_back(std::move(request));
        }

        result["partner_requests"] = J::Value(std::move(partner_requests));

        // Exam grades
        J::Array exam_grades;
        for (auto const& exam_grade : exam_grades_)
            exam_grades.push_back(exam_grade->to_json());
        result["exam_grades"] = J::Value(std::move(exam_grades));
    }
    return result;
}

dbo::ptr<User> User::find_this() const
{
    return session()->find<User>().where("id = ?").bind(id());
}

namespace rc = std::regex_constants;

static std::regex const admin_re("admin", rc::icase);
static std::regex const grader_re("grader", rc::icase);
static std::regex const student_re("student", rc::icase);

char const* Enum<User::Role>::show(User::Role role)
{
    switch (role) {
        case User::Role::Admin: return "admin";
        case User::Role::Grader: return "grader";
        case User::Role::Student: return "student";
    }
}

User::Role Enum<User::Role>::read(char const* role)
{
    if (std::regex_match(role, admin_re))
        return User::Role::Admin;

    if (std::regex_match(role, grader_re))
        return User::Role::Grader;

    if (std::regex_match(role, student_re))
        return User::Role::Student;

    throw std::invalid_argument{"Could not parse role"};
}


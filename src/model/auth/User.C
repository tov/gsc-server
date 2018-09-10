#include "User.h"
#include "Auth_token.h"
#include "../Assignment.h"
#include "../Eval_item.h"
#include "../File_data.h"
#include "../File_meta.h"
#include "../Grader_eval.h"
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

void User::add_auth_token(const std::string& value, const Wt::WDateTime& expires)
{

    if (auth_tokens_.size() > 50) return;

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

Wt::Json::Object User::to_json(bool brief) const
{
    Wt::Json::Object result;
    result["name"] = J::Value(name());
    result["uri"] = J::Value(rest_uri());
    if (!brief) {
        result["role"] = J::Value(role_string());
        Wt::Json::Array hws;
        for (const auto& each : submissions())
            hws.push_back(each->to_json(true));
        result["submissions"] = J::Value(hws);
    }
    return result;
}

const char* User::role_to_string(User::Role role)
{
    switch (role) {
        case User::Role::Admin: return "admin";
        case User::Role::Grader: return "grader";
        case User::Role::Student: return "student";
    }
}

namespace rc = std::regex_constants;

static std::regex const admin_re("admin", rc::icase);
static std::regex const grader_re("grader", rc::icase);
static std::regex const student_re("student", rc::icase);

User::Role User::string_to_role(std::string const& role)
{
    using namespace std;

    if (regex_match(role, admin_re)) {
        return User::Role::Admin;
    }

    if (regex_match(role, grader_re)) {
        return User::Role::Grader;
    }

    if (regex_match(role, student_re)) {
        return User::Role::Student;
    }

    throw invalid_argument{"Could not parse role"};
}

dbo::ptr<User> User::find_this() const
{
    return session()->find<User>().where("id = ?").bind(id());
}


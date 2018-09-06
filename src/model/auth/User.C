#include "../game/User_stats.h"
#include "User.h"
#include "Auth_token.h"
#include "../Assignment.h"
#include "../Eval_item.h"
#include "../File_data.h"
#include "../File_meta.h"
#include "../Grader_eval.h"
#include "../Self_eval.h"
#include "../Submission.h"

#include <Wt/Auth/PasswordHash.h>
#include <Wt/Dbo/Impl.h>
#include <Wt/Auth/Identity.h>

#include <algorithm>

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
    result.insert(result.end(), submissions1_.begin(), submissions1_.end());
    result.insert(result.end(), submissions2_.begin(), submissions2_.end());

    std::sort(result.begin(), result.end(),
              [&](const auto& s1, const auto& s2) {
        return s1->assignment()->number() < s2->assignment()->number();
    });

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

Wt::Json::Value User::to_json() const
{
    Wt::Json::Object result;
    result["name"] = J::Value(name());
    result["role"] = J::Value(role_string());
    return J::Value(result);
}

const char* User::role_to_string(User::Role role)
{
    switch (role) {
        case User::Role::Admin: return "admin";
        case User::Role::Grader: return "grader";
        case User::Role::Student: return "student";
    }
}

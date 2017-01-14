#pragma once

#include "../specializations.h"

#include <Wt/WDateTime>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/WtSqlTraits>

#include <string>

class Auth_token;
class Submission;
class User;
class User_stats;

namespace dbo = Wt::Dbo;

using Users       = dbo::collection<dbo::ptr<User>>;
using Auth_tokens = dbo::collection<dbo::ptr<Auth_token>>;
using Submissions = dbo::collection<dbo::ptr<Submission>>;

class User
{
public:
    User(const std::string& name = "");

    enum class Role : int
    {
        Student,
        Grader,
        Admin,
    };

    const std::string& name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

    Role role() const { return static_cast<Role>(role_); }
    void set_role(Role r) { role_ = static_cast<int>(r); }
    bool can_grade() const;
    bool can_admin() const;
    bool can_view(const dbo::ptr<User>&) const;

    Wt::Auth::PasswordHash password() const;
    void set_password(const Wt::Auth::PasswordHash&);

    Auth_tokens auth_tokens() const { return auth_tokens_; }
    void add_auth_token(const std::string&, const Wt::WDateTime&);
    void remove_auth_token(const std::string&);
    int update_auth_token(const std::string& old, const std::string& new_);

    int failed_login_attempts() const { return failed_login_attempts_; }
    void set_failed_login_attempts(int n) { failed_login_attempts_ = n; }

    const Wt::WDateTime& last_login_attempt() const
    { return last_login_attempt_; };

    void set_last_login_attempt(const Wt::WDateTime& date)
    { last_login_attempt_ = date; }

    std::vector<dbo::ptr<Submission>> submissions() const;
    dbo::weak_ptr<User_stats> user_stats() const { return user_stats_; }

    std::string hw_url() const;

    static dbo::ptr<User> find_by_name(dbo::Session&, const std::string&);
    static dbo::ptr<User> find_by_auth_token(dbo::Session&, const std::string&);

private:
    std::string name_;
    int role_ = static_cast<int>(Role::Student);

    std::string   password_;
    std::string   password_method_;
    std::string   password_salt_;
    int           failed_login_attempts_ = 0;
    Wt::WDateTime last_login_attempt_;
    Auth_tokens   auth_tokens_;

    Submissions   submissions1_;
    Submissions   submissions2_;

    dbo::weak_ptr<User_stats> user_stats_;

public:
    template<class Action>
    void persist(Action& a)
    {
        dbo::field(a, name_, "name", 16);
        dbo::field(a, role_, "role");

        dbo::field(a, password_, "password", 60);
        dbo::field(a, password_method_, "password_method", 6);
        dbo::field(a, password_salt_, "password_salt", 16);
        dbo::field(a, failed_login_attempts_, "failed_login_attempts");
        dbo::field(a, last_login_attempt_, "last_login_attempt");
        dbo::hasMany(a, auth_tokens_, dbo::ManyToOne, "user");

        dbo::hasMany(a, submissions1_, dbo::ManyToOne, "user1");
        dbo::hasMany(a, submissions2_, dbo::ManyToOne, "user2");

        dbo::hasOne(a, user_stats_, "user");
    }
};

DBO_EXTERN_TEMPLATES(User);


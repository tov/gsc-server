#pragma once

#include "../specializations.h"
#include "../../common/stringify.h"

#include <Wt/WDateTime.h>
#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Json/Object.h>
#include <Wt/WGlobal.h>

#include <string>

class Auth_token;
class Exam_grade;
class Submission;
class Partner_request;
class User;
class User_stats;

namespace dbo = Wt::Dbo;

using Auth_info        = Wt::Auth::Dbo::AuthInfo<User>;
using Exam_grades      = dbo::collection<dbo::ptr<Exam_grade>>;
using Exam_grades_vec  = std::vector<dbo::ptr<Exam_grade>>;
using Users            = dbo::collection<dbo::ptr<User>>;
using Auth_tokens      = dbo::collection<dbo::ptr<Auth_token>>;
using Submissions      = dbo::collection<dbo::ptr<Submission>>;
using Partner_requests = std::vector<dbo::ptr<Partner_request>>;

class User : public dbo::Dbo<User>
{
public:
    enum class Role : int
    {
        Student,
        Grader,
        Admin,
    };

    explicit User(const std::string& name = "",
                  Role role = Role::Student);

    const std::string& name() const { return name_; }

    void set_name(const std::string& name) { name_ = name; }

    Role role() const { return static_cast<Role>(role_); }
    void set_role(Role r) { role_ = static_cast<int>(r); }

    const char* role_string() const { return stringify(role()); }

    bool can_grade() const;
    bool can_admin() const;
    bool can_view(const dbo::ptr<User>&) const;

    std::vector<dbo::ptr<Submission>> submissions() const;

    Partner_requests outgoing_requests() const;
    Partner_requests incoming_requests() const;

    dbo::weak_ptr<User_stats> user_stats() const { return user_stats_; }
    Exam_grades_vec exam_grades() const;

    std::string hw_url() const;

    dbo::ptr<User> find_this() const;

    static dbo::ptr<User> find_by_name(dbo::Session&, const std::string&);

    std::string rest_uri() const;
    std::string submissions_rest_uri() const;
    Wt::Json::Object to_json(bool brief = false) const;

private:
    std::string name_;
    int role_ = static_cast<int>(Role::Student);

    Submissions submissions1_;
    Submissions submissions2_;

    dbo::collection<dbo::ptr<Partner_request>> outgoing_requests_;
    dbo::collection<dbo::ptr<Partner_request>> incoming_requests_;

    Exam_grades exam_grades_;
    dbo::weak_ptr<User_stats> user_stats_;

public:
    template<class Action>
    void persist(Action& a)
    {
        dbo::field(a, name_, "name", 16);
        dbo::field(a, role_, "role");

        dbo::hasMany(a, submissions1_, dbo::ManyToOne, "user1");
        dbo::hasMany(a, submissions2_, dbo::ManyToOne, "user2");

        dbo::hasMany(a, outgoing_requests_, dbo::ManyToOne, "requestor");
        dbo::hasMany(a, incoming_requests_, dbo::ManyToOne, "requestee");

        dbo::hasMany(a, exam_grades_, dbo::ManyToOne, "user");
        dbo::hasOne(a, user_stats_, "user");
    }
};

template <>
struct Enum<User::Role>
{
    static char const* show(User::Role);
    static User::Role read(char const*);
};

DBO_EXTERN_TEMPLATES(User)


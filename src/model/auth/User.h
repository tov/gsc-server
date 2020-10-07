#pragma once

#include "../../common/stringify.h"
#include "../specializations.h"

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Json/Object.h>
#include <Wt/WDateTime.h>
#include <Wt/WGlobal.h>

#include <optional>
#include <string>

#define MAX_CALL_ME_LENGTH 64

struct Auth_token;
class Exam_grade;
class Submission;
class Partner_request;
class User;
class User_stats;

namespace dbo = Wt::Dbo;

using Auth_info = Wt::Auth::Dbo::AuthInfo<User>;
using Exam_grades = dbo::collection<dbo::ptr<Exam_grade>>;
using Exam_grades_vec = std::vector<dbo::ptr<Exam_grade>>;
using Users = dbo::collection<dbo::ptr<User>>;
using Auth_tokens = dbo::collection<dbo::ptr<Auth_token>>;
using Submissions = dbo::collection<dbo::ptr<Submission>>;
using Partner_requests_vec = std::vector<dbo::ptr<Partner_request>>;
using Partner_requests = dbo::collection<dbo::ptr<Partner_request>>;

class User : public dbo::Dbo<User> {
public:
  enum class Role : int {
    Student,
    Grader,
    Admin,
  };

  struct Role_info {
    static constexpr size_t N = 3;
    using info_t = Enum_info<Role, N>;

    static info_t const info;

    static char const *show(Role);
    static Role read(char const *);

    static int to_repr(Role);
    static Role from_repr(int, Role);
    static std::optional<Role> from_repr(int);
  };

  explicit User(const std::string &name = "", Role role = Role::Student);

  const std::string &name() const { return name_; }

  void set_name(const std::string &name) { name_ = name; }

  Role role() const { return Role_info::from_repr(role_, Role::Student); }
  void set_role(Role r) { role_ = Role_info::to_repr(r); }

  const char *role_string() const { return stringify(role()); }

  std::string const &call_me() const { return call_me_; }
  void set_call_me(std::string const &);

  bool can_grade() const;
  bool can_admin() const;
  bool can_view(const dbo::ptr<User> &) const;
  bool can_view(const std::string &) const;

  void check_can_grade() const;
  void check_can_admin() const;
  void check_can_view(const dbo::ptr<User> &) const;
  void check_can_view(const std::string &) const;

  std::vector<dbo::ptr<Submission>> submissions() const;

  Partner_requests_vec outgoing_requests() const;
  Partner_requests_vec incoming_requests() const;

  dbo::weak_ptr<User_stats> user_stats() const { return user_stats_; }
  Exam_grades_vec exam_grades() const;

  std::string hw_url() const;
  std::string profile_url() const;

  dbo::ptr<User> find_this() const;

  static dbo::ptr<User> find_by_name(dbo::Session &, const std::string &);

  std::string rest_uri() const;
  std::string submissions_rest_uri() const;
  Wt::Json::Object to_json(bool brief = false) const;

private:
  std::string name_;
  int role_ = Role_info::to_repr(Role::Student);

  std::string call_me_;

  Submissions submissions1_;
  Submissions submissions2_;

  Partner_requests outgoing_requests_;
  Partner_requests incoming_requests_;

  Exam_grades exam_grades_;
  dbo::weak_ptr<User_stats> user_stats_;

public:
  template <class Action> void persist(Action &a) {
    dbo::field(a, name_, "name", 16);
    dbo::field(a, role_, "role");

    dbo::field(a, call_me_, "call_me", MAX_CALL_ME_LENGTH);

    dbo::hasMany(a, submissions1_, dbo::ManyToOne, "user1");
    dbo::hasMany(a, submissions2_, dbo::ManyToOne, "user2");

    dbo::hasMany(a, outgoing_requests_, dbo::ManyToOne, "requestor");
    dbo::hasMany(a, incoming_requests_, dbo::ManyToOne, "requestee");

    dbo::hasMany(a, exam_grades_, dbo::ManyToOne, "user");
    dbo::hasOne(a, user_stats_, "user");
  }
};

template <> struct Enum<User::Role> : User::Role_info {};

DBO_EXTERN_TEMPLATES(User)

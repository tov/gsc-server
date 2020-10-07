#pragma once

#include "model/auth/Environment.h"
#include "model/auth/User.h"

#include <Wt/Auth/AbstractUserDatabase.h>
#include <Wt/Auth/Dbo/UserDatabase.h>
#include <Wt/Auth/Identity.h>
#include <Wt/Auth/Login.h>

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/Dbo/ptr.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

class User;
class User_stats;

using User_database = Wt::Auth::Dbo::UserDatabase<Auth_info>;

#define FORWARD_TO(NAME, RCVR)                                                 \
  template <typename T, typename... Args> auto NAME(Args &&... args) {         \
    return (RCVR).NAME<T>(std::forward<Args>(args)...);                        \
  }

class Db_session;

template <typename T> struct authn_result;

template <typename T> using authn_result_t = typename authn_result<T>::type;

template <typename T>
authn_result_t<T> lift_authn_result(Wt::Auth::User const &auth_user,
                                    Db_session const &session_) {
  return authn_result<T>::lift(auth_user, session_);
}

template <> struct authn_result<Wt::Auth::User> {
  using type = Wt::Auth::User;
  static type lift(Wt::Auth::User const &, Db_session const &);
};

template <> struct authn_result<Auth_info> {
  using type = Wt::Dbo::ptr<Auth_info>;
  static type lift(Wt::Auth::User const &, Db_session const &);
};

template <> struct authn_result<User> {
  using type = Wt::Dbo::ptr<User>;
  static type lift(Wt::Auth::User const &, Db_session const &);
};

struct User_auth_params {
  //
  // Member variables
  //

  std::string username;
  User::Role role = User::Role::Student;
#ifdef GSC_AUTH_PASSWORD
  std::string password;
#endif // GSC_AUTH_PASSWORD
};

class Db_session {
public:
  using dbo_t = Wt::Dbo::Session;
  using auth_user_t = Wt::Auth::User;

  explicit Db_session(Wt::Dbo::SqlConnectionPool &);
  explicit Db_session(std::unique_ptr<Wt::Dbo::SqlConnection>);

  dbo_t &dbo() const { return dbo_; };
  operator dbo_t &() const { return dbo(); }

  FORWARD_TO(addNew, dbo())
  FORWARD_TO(find, dbo())
  FORWARD_TO(query, dbo())
  FORWARD_TO(mapClass, dbo())

  User_database &users() { return *users_; }

  const User_database &users() const { return *users_; }

  std::pair<Wt::Dbo::ptr<User>, Wt::Auth::User>
  create_user(const User_auth_params &);

  template <typename T>
  authn_result_t<T> find_by_identity(const std::string &provider,
                                     const std::string &identity) const {
    Wt::Dbo::Transaction transaction(*this);
    auto auth_user = users().findWithIdentity(provider, identity);
    return lift_authn_result<T>(auth_user, *this);
  }

  template <typename T>
  authn_result_t<T> find_by_auth_token(const std::string &token) const {
    auto auth_user = users().findWithAuthToken(token);
    return lift_authn_result<T>(auth_user, *this);
  }

  template <typename T>
  authn_result_t<T> find_by_login(const std::string &username,
                                  bool create = true) {
    auto provider = Wt::Auth::Identity::LoginName;
    auto auth_user = find_by_identity<auth_user_t>(provider, username);

    if (!auth_user.isValid() && create)
      auth_user = create_user({username}).second;

    return lift_authn_result<T>(auth_user, *this);
  }

  template <typename T>
  authn_result_t<T>
  find_from_environment(bool create = true,
                        Environment const &env = App_environment()) {
    if (auto remote_user = env_remote_user(env)) {
      return find_by_login<T>(*remote_user, create);
    } else {
      return {};
    }
  }

#ifdef GSC_AUTH_API_KEY
  void set_api_key(dbo::ptr<User> const &);
  std::string get_api_key(dbo::ptr<User> const &);
#endif // GSC_AUTH_API_KEY

#ifdef GSC_AUTH_PASSWORD
  void set_password(dbo::ptr<User> const &user,
                    std::string const &password) const;
#endif // GSC_AUTH_PASSWORD

  std::vector<dbo::ptr<User_stats>> top_users(int limit);

  void map_classes();

  void initialize_db(bool test_data = true);

  static void configure_auth();

  static std::unique_ptr<Wt::Dbo::SqlConnectionPool>
  createConnectionPool(const std::string &);

  static const Wt::Auth::AuthService &auth();
#ifdef GSC_AUTH_PASSWORD
  static const Wt::Auth::AbstractPasswordService &passwordAuth();
#endif // GSC_AUTH_PASSWORD

private:
  std::unique_ptr<User_database> users_;
  mutable dbo_t dbo_;

  void create_index_(const char *table, const char *field,
                     bool unique = true) const;

  void initialize_session_();
  void populate_test_data_();
};

class Session : public Db_session {
public:
  using Db_session::Db_session;

  Wt::Auth::Login &login() { return login_; }

  Wt::Dbo::ptr<User> user() const;

  std::string user_name() const;

  void become_user(const Wt::Dbo::ptr<User> &);

  int find_ranking();
  void add_to_score(int s);

  bool authenticate_from_environment(Environment const & = App_environment{});

private:
  Wt::Auth::Login login_;

  // `mutable` means cache (may be nullptr):
  mutable Wt::Dbo::ptr<Auth_info> auth_info_;
};

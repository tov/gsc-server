#pragma once

#include "model/auth/User_database.h"
#include "model/auth/User.h"

#include <Wt/Auth/Login.h>
#include <Wt/Auth/AbstractUserDatabase.h>

#include <Wt/Dbo/Session.h>
#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/SqlConnectionPool.h>

#include <memory>
#include <string>
#include <vector>

class User;
class User_stats;

class Db_session : public Wt::Dbo::Session
{
public:
    Db_session(Wt::Dbo::SqlConnectionPool&);
    Db_session(std::unique_ptr<Wt::Dbo::SqlConnection>);

    Wt::Dbo::Session& dbo() { return *this; };

    User_database& users() { return users_; }

    const User_database& users() const { return users_; }

    Wt::Dbo::ptr<User> create_user(const std::string& username,
                                   const std::string& password,
                                   User::Role role = User::Role::Student);
    void set_password(const dbo::ptr<User>& user,
                      const std::string& password);

    std::vector<dbo::ptr<User_stats>> top_users(int limit);

    void map_classes();

    static void initialize_db(Wt::Dbo::SqlConnectionPool&);

    static void configure_auth();

    static std::unique_ptr<Wt::Dbo::SqlConnectionPool>
    createConnectionPool(const std::string&);

    static const Wt::Auth::AuthService& auth();

    static const Wt::Auth::AbstractPasswordService& passwordAuth();

private:
    User_database users_;

    void create_index_(const char* table, const char* field, bool unique = true);

    void initialize_db_();
};

class Session : public Db_session
{
public:
    using Db_session::Db_session;

    Wt::Auth::Login& login() { return login_; }

    Wt::Dbo::ptr<User> user() const;

    std::string user_name() const;

    void become_user(const Wt::Dbo::ptr<User>&);

    int find_ranking();
    void add_to_score(int s);

private:
    Wt::Auth::Login login_;

    mutable Wt::Dbo::ptr<User> user_;
};


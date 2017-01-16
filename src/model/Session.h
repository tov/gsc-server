#pragma once

#include "auth/User_database.h"

#include <Wt/Auth/Login>
#include <Wt/Auth/AbstractUserDatabase>

#include <Wt/Dbo/Session>
#include <Wt/Dbo/ptr>
#include <Wt/Dbo/SqlConnectionPool>

#include <vector>

class User;
class User_stats;

class Session : public Wt::Dbo::Session
{
public:
    Session(Wt::Dbo::SqlConnectionPool&);

    Wt::Auth::AbstractUserDatabase& users();

    Wt::Auth::Login& login() { return login_; }

    Wt::Dbo::Session& dbo() { return *this; };

    Wt::Dbo::ptr<User> user() const;

    std::string user_name() const;

    std::vector<dbo::ptr<User_stats>> top_users(int limit);
    int find_ranking();
    void add_to_score(int s);

    static void map_classes(Wt::Dbo::Session&);

    static void configureAuth();
    static Wt::Dbo::SqlConnectionPool* createConnectionPool(const std::string&);
    static const Wt::Auth::AuthService& auth();
    static const Wt::Auth::AbstractPasswordService& passwordAuth();

private:
    User_database users_;
    Wt::Auth::Login login_;

    mutable Wt::Dbo::ptr<User> user_;

    void create_index(const char* table, const char* field, bool unique = true);
};


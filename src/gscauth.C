#include "model/auth/User.h"
#include "common/env_var.h"
#include "Session.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/Identity.h>
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordVerifier.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

namespace dbo = Wt::Dbo;

using namespace std;

class Gscauth
{
public:
    Gscauth();

    bool check_user(User::Role role, const string& username, const string& password);

    dbo::Session& session() { return session_; }

private:
    Db_session session_{get_db_conn_()};

    dbo::ptr<User> find_user_(const string&);

    static const char* get_db_string_();
    static std::unique_ptr<dbo::SqlConnection> get_db_conn_();
};

void assert_argc(bool okay, const char* argv[], const char* message)
{
    if (!okay) {
        cerr << "Usage: " << argv[0] << ' '
                  << argv[1] << ' ' << message << '\n';
        exit(2);
    }
}

int main(int argc, const char* argv[])
{
    User::Role role;

    switch (argc) {
        case 1:
            role = User::Role::Grader;
            break;
        case 2:
            if (std::strcmp(argv[1], "-a") == 0) {
                role = User::Role::Admin;
                break;
            }
        default:
            cerr << "Usage: " << argv[0] << " [-a]...\n";
            exit(1);
    }

    Gscauth app;
    dbo::Transaction transaction(app.session());

    string username, password;
    getline(cin, username);
    getline(cin, password);

    if (app.check_user(role, username, password))
        return 0;
    else
        return 1;
}

Gscauth::Gscauth()
{
    Db_session::configure_auth();
}

bool Gscauth::check_user(User::Role role, const string& username, const string& password)
{
    auto user = find_user_(username);

    dbo::Transaction transaction(session_);

    auto& service      = Db_session::passwordAuth();
    auto auth_user     = session_.users().find(user);
    auto verify_result = service.verifyPassword(auth_user, password);

    transaction.rollback();

    switch (verify_result) {
        case Wt::Auth::PasswordResult::PasswordValid:
            break;

        case Wt::Auth::PasswordResult::PasswordInvalid:
            cerr << "Password invalid\n";
            return false;

        case Wt::Auth::PasswordResult::LoginThrottling:
            cerr << "Too many attempts in too little time\n";
            return false;

        default:
            cerr << "No good!\n";
            return false;
    }

    if (role > user->role()) {
        cerr << "User does not have required role\n";
        return false;
    }

    return true;
}

dbo::ptr<User> Gscauth::find_user_(const string& username)
{
    auto user = User::find_by_name(session(), username);
    if (!user) {
        cerr << "User not found: " << username << '\n';
        exit(1);
    }
    return user;
}

const char* Gscauth::get_db_string_()
{
    return get_env_var("POSTGRES_CONNINFO", "dbname=gsc");
}

std::unique_ptr<dbo::SqlConnection> Gscauth::get_db_conn_()
{
    return make_unique<dbo::backend::Postgres>(get_db_string_());
}


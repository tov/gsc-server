#include "view/cli/Cli_base.h"
#include "model/auth/User.h"
#include "Session.h"

#ifdef GSC_AUTH_PASSWORD
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordVerifier.h>
#endif

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include <syslog.h>

#define BAD_IDENTITY  1
#define BAD_ROLE      2
#define BAD_API_KEY   4
#define BAD_PASSWORD  8

namespace dbo = Wt::Dbo;

using namespace std;

class Gsc_auth_app : public cli::Cli_base
{
public:
    static int main(int argc, const char* argv[],
                    istream&, ostream&, ostream&);

private:
    Gsc_auth_app();
    int check_user_(User::Role role,
                    const string& identity,
                    const string& secret);
};


int main(int argc, const char* argv[])
{
    return Gsc_auth_app::main(argc, argv, cin, cout, cerr);
}


Gsc_auth_app::Gsc_auth_app()
{
    Db_session::configure_auth();
}

int Gsc_auth_app::main(int argc, const char* argv[],
                       istream& in, ostream& out, ostream& err)
{
    User::Role role;

    switch (argc) {
        case 1:
            role = User::Role::Grader;
            break;

        case 2:
            if (strcmp(argv[1], "-a") == 0) {
                role = User::Role::Admin;
                break;
            }

        default:
            err << "Usage: " << argv[0] << " [-a]...\n";
            exit(1);
    }

    string username, secret;
    getline(in, username);
    getline(in, secret);

    Gsc_auth_app app;
    return app.check_user_(role, username, secret);
}

namespace {

void log_auth_failure(std::string const& identity, char const* reason)
{
    openlog("gsc-auth", 0, LOG_AUTH);
    syslog(LOG_NOTICE, "user %s: %s", identity.c_str(), reason);
    closelog();
}

}  // anonymous namespace

int Gsc_auth_app::check_user_(
        User::Role role, const string& identity, const string& secret)
{
    dbo::Transaction transaction(session());

    auto auth_info = session().find_by_login<Auth_info>(identity, false);
    if (!auth_info) {
        log_auth_failure(identity, "does not exist");
        return BAD_IDENTITY;
    }

    auto auth_user = session().users().find(auth_info);
    auto user      = auth_info->user();
    int result     = 0;

    if (user->role() < role) {
        log_auth_failure(identity, "not authorized for role");
        return BAD_ROLE;
    }

#ifdef GSC_AUTH_API_KEY
    if (secret == session().get_api_key(user)) {
        return 0;
    } else {
        log_auth_failure(identity, "API KEY mismatch");
        result |= BAD_API_KEY;
    }
#endif

#ifdef GSC_AUTH_PASSWORD
    if (auto verify_result = Db_session::passwordAuth().verifyPassword(auth_user, secret);
            verify_result == Wt::Auth::PasswordResult::PasswordValid) {
        return 0;
    } else {
        log_auth_failure(identity, "password mismatch");
        result |= BAD_PASSWORD;
    }
#endif

    return result;
}



#include "Session.h"
#include "model/auth/User.h"
#include "view/cli/Cli_base.h"

#ifdef GSC_AUTH_PASSWORD
#include <Wt/Auth/PasswordService.h>
#include <Wt/Auth/PasswordVerifier.h>
#endif

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

namespace dbo = Wt::Dbo;

using namespace std;

class Gsc_auth_app : public cli::Cli_base {
public:
  static int main(int argc, const char *argv[], istream &, ostream &,
                  ostream &);

private:
  Gsc_auth_app();
  bool check_user_(User::Role role, const string &identity,
                   const string &secret);
};

int main(int argc, const char *argv[]) {
  return Gsc_auth_app::main(argc, argv, cin, cout, cerr);
}

Gsc_auth_app::Gsc_auth_app() { Db_session::configure_auth(); }

int Gsc_auth_app::main(int argc, const char *argv[], istream &in, ostream &out,
                       ostream &err) {
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
  return app.check_user_(role, username, secret) ? 0 : 1;
}

bool Gsc_auth_app::check_user_(User::Role role, const string &identity,
                               const string &secret) {
  dbo::Transaction transaction(session());

  auto auth_info = session().find_by_login<Auth_info>(identity, false);
  auto auth_user = session().users().find(auth_info);
  auto user = auth_info->user();

  if (user->role() < role)
    return false;

#ifdef GSC_AUTH_API_KEY
  if (secret == session().get_api_key(user))
    return true;
#endif

#ifdef GSC_AUTH_PASSWORD
  if (auto verify_result =
          Db_session::passwordAuth().verifyPassword(auth_user, secret);
      verify_result == Wt::Auth::PasswordResult::PasswordValid)
    return true;
#endif

  return false;
}

#include "Session.h"
#include "view/cli/Cli_base.h"

#include <iostream>
#include <memory>

namespace dbo = Wt::Dbo;

using namespace std;

struct Gsc_createdb_app : public cli::Cli_base {
  Gsc_createdb_app() : Cli_base(true) {}

  static int main(int argc, const char *argv[], istream &, ostream &,
                  ostream &);
};

int main(int argc, const char *argv[]) {
  return Gsc_createdb_app::main(argc, argv, cin, cout, cerr);
}

int Gsc_createdb_app::main(int argc, char const **argv, istream &in,
                           ostream &out, ostream &err) {
  bool test_data = false;

  switch (argc) {
  case 1:
    break;

  case 2:
    if (strcmp(argv[1], "-c") == 0) {
      test_data = true;
      break;
    }

  default:
    err << "Usage: " << argv[0] << " [-c]...\n";
    exit(1);
  }

  Gsc_createdb_app app;

  try {
    app.session().initialize_db(test_data);
  } catch (...) {
    return 1;
  }

  return 0;
}

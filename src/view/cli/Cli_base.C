#include "../../common/env_var.h"
#include "Cli_base.h"

#include <Wt/Dbo/backend/Postgres.h>
#include <Wt/WLogger.h>

#include <memory>

namespace dbo = Wt::Dbo;

namespace cli {

Cli_base::Cli_base(bool show_queries) : session_(get_db_conn_(show_queries)) {
  Db_session::configure_auth();
}

dbo::ptr<Auth_info> Cli_base::find_user(std::string const &username) {
  if (auto user = session_.find_by_login<Auth_info>(username))
    return user;

  throw User_not_found(username);
}

const char *Cli_base::get_db_string_() {
  return get_env_var("POSTGRES_CONNINFO", "dbname=gsc");
}

std::unique_ptr<dbo::SqlConnection> Cli_base::get_db_conn_(bool show_queries) {
  auto result = std::make_unique<dbo::backend::Postgres>(get_db_string_());

  if (show_queries)
    result->setProperty("show-queries", "true");

  return result;
}

User_not_found::User_not_found(std::string const &user_name)
    : std::runtime_error{"User not found: " + user_name}, user_name_{
                                                              user_name} {}

std::string const &User_not_found::user_name() { return user_name_; }

} // end namespace cli

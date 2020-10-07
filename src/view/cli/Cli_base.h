#pragma once

#include "../../Session.h"

#include <Wt/Dbo/SqlConnection.h>
#include <Wt/Dbo/ptr.h>

#include <memory>
#include <stdexcept>
#include <string>

namespace cli {

class User_not_found : std::runtime_error {
public:
  explicit User_not_found(std::string const &);
  std::string const &user_name();

private:
  std::string user_name_;
};

class Cli_base {
public:
  explicit Cli_base(bool show_queries = false);

protected:
  Db_session &session() { return session_; }

  Wt::Dbo::ptr<Auth_info> find_user(std::string const &);

private:
  Db_session session_;

  static const char *get_db_string_();

  static std::unique_ptr<Wt::Dbo::SqlConnection>
  get_db_conn_(bool show_queries);
};

} // end namespace cli

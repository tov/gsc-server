#pragma once

#include <string>

struct Config {
  float reward_for_no;
  bool auto_create_accounts;
  bool show_queries;
  std::string gsc_base_url;
  std::string server_time_zone;
  std::string websso_login_url;
  std::string websso_logout_url;

  Config();
};

extern const Config CONFIG;

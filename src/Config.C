#include "Config.h"

const Config CONFIG;

Config::Config()
    : reward_for_no{0.5}
    , auto_create_accounts{true}
    , show_queries{false}
    , gsc_base_url{"http://localhost:9090/"}
    , server_time_zone{"America/Chicago"}
    , websso_login_url{"https://prd-nusso.it.northwestern.edu/nusso/XUI/#login"}
    , websso_logout_url{"https://prd-nusso.it.northwestern.edu/nusso/XUI/"
                        "#logout"}
    , gsc_repo{"../../var/"}
    , git_lockfile{"gitlock"}
{}

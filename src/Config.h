#pragma once

#include <string>

struct Config
{
    float reward_for_no = 0.2;

    bool show_queries = false;

    std::string websso_login_url
            = "https://websso.it.northwestern.edu/amserver/UI/Login";

    std::string websso_logout_url
            = "https://websso.it.northwestern.edu/amserver/UI/Logout";

    std::string gsc_base_url = "http://localhost:9090/";

    std::string server_time_zone = "America/Chicago";
};

extern Config const CONFIG;

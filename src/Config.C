#include "Config.h"
#include "common/env_var.h"
#include "common/util.h"

#include <Wt/Json/Object.h>
#include <Wt/Json/Parser.h>
#include <Wt/Json/Value.h>

#include <algorithm>
#include <fstream>
#include <iterator>

#ifndef GSCD_CONFIG_ENV_VAR
#  define GSCD_CONFIG_ENV_VAR "GSCD_CONFIG_JSON"
#endif

#ifndef GSCD_CONFIG_DEFAULT
#  define GSCD_CONFIG_DEFAULT "gscd-config.json"
#endif

namespace {

char const* config_file()
{
    return get_env_var(GSCD_CONFIG_ENV_VAR, GSCD_CONFIG_DEFAULT);
}

void read_all(istream& in, string& out)
{
    using iter = istreambuf_iterator<char>;
    copy(iter(in), iter(), back_inserter(out));
}

void parse(istream& in, J::Object& out)
{
    string buf;

    read_all(in, buf);
    J::parse(buf, out);
}

unique_ptr<J::Object> load_json()
{
    auto result = make_unique<J::Object>();

    if (ifstream infile(config_file()); infile) {
        parse(infile, *result);
    }

    return result;
}

}  // end anonymous namespace


Config const& CONFIG()
{
    static Config instance;
    return instance;
}

J::Value const& Config::get(string const& key) const
{
    return json_->get(key);
}

Config::Config() : json_(load_json())
{
    set_daily_discount_();
    set_reward_for_no_();
    set_auto_create_accounts_();
    set_show_queries_();
    set_display_file_quota_();
    set_instance_class_();
    set_instance_name_();
    set_gsc_base_url_();
    set_postgres_conninfo_();
    set_server_time_zone_();
    set_websso_login_url_();
    set_websso_logout_url_();
}

#include "Environment.h"
#include "../../common/env_var.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"

#ifdef GSC_AUTH_OPEN_AM
namespace {

struct Param
{
    std::string hdr, var;
    std::string lookup(Wt::WEnvironment const&) const;
};

Param const auth_type_param   { "X-GSC-Auth-Type",   "AUTH_TYPE" };
Param const remote_user_param { "X-GSC-Remote-User", "REMOTE_USER" };

std::string const open_am_auth_type = "OpenAM";

std::string Param::lookup(Wt::WEnvironment const& env) const
{
    auto value = env.headerValue(hdr);
    return value.empty() ? get_env_var(var) : value;
}

}
#endif // GSC_AUTH_OPEN_AM

std::optional<std::string> env_remote_user()
{
    auto app = Wt::WApplication::instance();
    if (!app) return {};

    auto const& env = app->environment();

#ifdef GSC_AUTH_DEBUG
    if (auto whoami = env.getParameter("whoami"))
        return std::make_optional(*whoami);
#endif // GSC_AUTH_DEBUG

#ifdef GSC_AUTH_OPEN_AM
    if (auth_type_param.lookup(env) != open_am_auth_type)
        return {};

    if (auto username = remote_user_param.lookup(env);
            ! username.empty())
        return std::make_optional(username);
#endif // GSC_AUTH_OPEN_AM

    return {};
}


#include "Environment.h"
#include "../../common/env_var.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"

namespace {

struct Param
{
    std::string hdr, var;
    std::string lookup() const;
};

Param const auth_type_param   { "X-GSC-Auth-Type",   "AUTH_TYPE" };
Param const remote_user_param { "X-GSC-Remote-User", "REMOTE_USER" };

std::string const open_am_auth_type = "OpenAM";

std::string Param::lookup() const
{
    auto const& env = Wt::WApplication::instance()->environment();
    auto value = env.headerValue(hdr);
    return value.empty() ? get_env_var(var) : value;
}

}

std::optional<std::string> env_remote_user()
{
#ifdef GSC_AUTH_OPEN_AM
    if (auth_type_param.lookup() != open_am_auth_type)
        return {};

    if (auto username = remote_user_param.lookup();
            ! username.empty())
        return std::make_optional(username);
#endif // GSC_AUTH_OPEN_AM

    return {};
}


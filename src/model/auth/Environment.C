#include "Environment.h"
#include "../../common/env_var.h"

namespace {

std::string const open_am_auth_var   = "AUTH_TYPE";
std::string const open_am_auth_value = "OpenAM";
std::string const open_am_user_var   = "REMOTE_USER";

}

std::optional<std::string> env_remote_user()
{
#ifdef GSC_AUTH_OPEN_AM
    if (get_env_var(open_am_auth_var) != open_am_auth_value)
        return {};

    if (auto username = get_env_var(open_am_user_var, nullptr))
        return std::make_optional<std::string>(username);
#endif // GSC_AUTH_OPEN_AM

    return {};
}


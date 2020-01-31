#include "Environment.h"
#include "../../common/env_var.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"

using std::string;

#ifdef GSC_AUTH_OPEN_AM
namespace {

struct Param
{
    string hdr, var;
    string lookup(Wt::WEnvironment const&) const;
};

Param const auth_type_param   { "X-GSC-Auth-Type",   "AUTH_TYPE" };
Param const remote_user_param { "X-GSC-Remote-User", "REMOTE_USER" };

string const open_am_auth_type = "OpenAM";

string Param::lookup(Wt::WEnvironment const& env) const
{
    auto value = env.headerValue(hdr);
    return value.empty() ? get_env_var(var) : value;
}

}
#endif // GSC_AUTH_OPEN_AM

std::optional<string> env_remote_user()
{
    auto app = Wt::WApplication::instance();
    if (!app) return {};

    auto const& env = app->environment();

#ifdef GSC_AUTH_DEBUG
    if (string const* whoami = env.getParameter("whoami")) {
        return {*whoami};
    } else {
        std::ostringstream target;
        target << '/' << app->bookmarkUrl() << "?whoami=admin";
        app->redirect(target.str());
        app->quit();
    }
#endif

#ifdef GSC_AUTH_OPEN_AM
    if (auth_type_param.lookup(env) == open_am_auth_type) {
        if (string username = remote_user_param.lookup(env);
                !username.empty())
        {
            return {username};
        }
    }
#endif // GSC_AUTH_OPEN_AM

    return {};
}


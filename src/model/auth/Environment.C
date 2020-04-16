#include "Environment.h"
#include "../../common/env_var.h"

#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"

using namespace std;

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

optional<string> param_whoami()
{
    auto app = Wt::WApplication::instance();
    if (!app) return nullopt;

    string const* whoami = app->environment().getParameter("whoami");
    if (whoami)
        return {*whoami};
    else
        return nullopt;
}

void redirect_with_whoami(std::string const& uri,
                          std::string const& whoami)
{
    auto app = Wt::WApplication::instance();
    if (!app) return;

    ostringstream target;
    target << '/' << uri << "?whoami=" << whoami;
    app->redirect(target.str());
    app->quit();
}

void redirect_with_whoami(string const& whoami)
{
    auto app = Wt::WApplication::instance();
    if (!app) return;
    redirect_with_whoami(app->bookmarkUrl(), whoami);
}

optional<string> env_remote_user()
{
#ifdef GSC_AUTH_DEBUG
    if (auto whoami = param_whoami())
        return whoami;
    else
        redirect_with_whoami("admin");
#endif

#ifdef GSC_AUTH_OPEN_AM
    auto app = Wt::WApplication::instance();
    if (!app) return nullopt;

    auto const& env = app->environment();

    if (auth_type_param.lookup(env) != open_am_auth_type) return nullopt;

    string username = remote_user_param.lookup(env);
    if (!username.empty()) return optional(username);
#endif // GSC_AUTH_OPEN_AM

    return nullopt;
}


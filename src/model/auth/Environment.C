#include "../../common/util.h"
#include "Environment.h"

#include "Wt/WApplication.h"

using namespace std;

static WEnvironment const *app_env() {
  if (auto app = Wt::WApplication::instance())
    return &app->environment();
  else
    return nullptr;
}

App_environment::App_environment() : environment_(app_env()) {}

bool App_environment::invalid() const { return environment_ == nullptr; }

string const *App_environment::get_parameter(string const &name) const {
  return environment_ ? environment_->getParameter(name) : nullptr;
}

string App_environment::header_value(string const &name) const {
  return environment_ ? environment_->headerValue(name) : "";
}

Req_environment::Req_environment(Http::Request const &request)
    : request_(request) {}

bool Req_environment::invalid() const { return false; }

string const *Req_environment::get_parameter(string const &name) const {
  return request_.getParameter(name);
}

string Req_environment::header_value(string const &name) const {
  return request_.headerValue(name);
}

namespace {

struct Param {
  string hdr, var;

  string lookup(Environment const &) const;
};

string Param::lookup(Environment const &env) const {
  auto value = env.header_value(hdr);
  return value.empty() ? get_env_var(var) : value;
}

const Param auth_type_param{"X-GSC-Auth-Type", "AUTH_TYPE"};
const Param remote_user_param{"X-GSC-Remote-User", "REMOTE_USER"};

string const open_am_auth_type = "OpenAM";

} // namespace

optional<string> param_whoami(Environment const &env) {
  if (!env)
    return nullptr;

  if (auto whoami = env.get_parameter("whoami"))
    return {*whoami};
  else
    return nullopt;
}

optional<string> env_remote_user(Environment const &env) {
  if (!env)
    return nullptr;

#ifdef GSC_AUTH_DEBUG
  if (auto whoami = param_whoami(env))
    return whoami;
  else
    redirect_with_whoami("admin");
#endif

#ifdef GSC_AUTH_OPEN_AM
  if (auth_type_param.lookup(env) != open_am_auth_type)
    return nullopt;

  if (string username = remote_user_param.lookup(env); !username.empty())
    return {username};
#endif // GSC_AUTH_OPEN_AM

  return nullopt;
}

void redirect_with_whoami(string const &uri, string const &whoami) {
  auto app = Wt::WApplication::instance();
  if (!app)
    return;

  ostringstream target;
  target << '/' << uri << "?whoami=" << whoami;
  app->redirect(target.str());
  app->quit();
}

void redirect_with_whoami(string const &whoami) {
  auto app = Wt::WApplication::instance();
  if (!app)
    return;
  redirect_with_whoami(app->bookmarkUrl(), whoami);
}

#include "../../common/env_var.h"
#include "../../common/stringify.h"
#include "../../model/auth/Api_key.h"
#include "../../model/auth/Environment.h"
#include "../Http_status.h"
#include "Request_handler.h"

#include <Wt/Auth/AuthService.h>
#include <Wt/Auth/HashFunction.h>
#ifdef GSC_AUTH_PASSWORD
#include <Wt/Auth/PasswordStrengthValidator.h>
#endif // GSC_AUTH_PASSWORD

#include <Wt/Http/Response.h>
#include <Wt/Utils.h>
#include <Wt/WDateTime.h>

#include <sstream>

namespace api {

#ifdef GSC_AUTH_PASSWORD
static const std::regex authorization_pat(" *basic ([^ ]+) *",
                                          std::regex_constants::icase);

static Credentials parse_authorization(const std::string &header_value) {
  std::smatch sm;

  if (header_value.empty())
    return Credentials{"", ""};

  if (!std::regex_match(header_value, sm, authorization_pat)) {
    throw Http_status{400, "Could not parse authorization header"};
  }

  std::string encoded(sm[1].first, sm[1].second);
  std::string decoded = Wt::Utils::base64Decode(encoded);
  auto colon = std::find(decoded.begin(), decoded.end(), ':');

  if (colon == decoded.end()) {
    throw Http_status{400, "Could not parse decoded authorization header"};
  }

  std::string username(decoded.begin(), colon);
  std::string password(colon + 1, decoded.end());
  return Credentials{username, password};
}
#endif // GSC_AUTH_PASSWORD

Request_handler::Request_handler(Db_session &session,
                                 Wt::Http::Request const &request)
    : session_{session}, request_{request},
      path_info_{request_.pathInfo()}, method_{request.method()} {
  if (path_info_.substr(0, 4) == "/api") {
    path_info_ = path_info_.substr(4);
  }
}

dbo::ptr<User> Request_handler::authenticate() const {
  dbo::ptr<User> user;

  if ((user = authenticate_by_api_key_()))
    return user;
  if ((user = authenticate_by_cookie_()))
    return user;
  if ((user = authenticate_by_open_am_()))
    return user;
  if ((user = authenticate_by_password_()))
    return user;

  throw Http_status{401, "Access to this resource requires authentication"};
}

std::unique_ptr<resources::Resource> Request_handler::parse_uri() const {
  return resources::Resource::create(method_, path_info_);
}

#ifdef GSC_AUTH_COOKIE
static std::string const cookie_name = "gsc_cookie";

void Request_handler::set_cookie_(std::string const &value,
                                  int ttl_seconds) const {
  auto expiry = Wt::WDateTime::currentDateTime().addSecs(ttl_seconds);

  std::ostringstream out_cookie;
  out_cookie << cookie_name << "=" << value << "; Path=/api"
             << "; Expires="
             << expiry.toString("ddd, d MMM yyyy hh:mm:ss 'GMT'");

  response_.addHeader("Set-Cookie", out_cookie.str());
}

void Request_handler::create_cookie_(Wt::Auth::User const &auth_user) const {
  std::string auth_token = session_.auth().createAuthToken(auth_user);
  set_cookie_(auth_token, 60 * session_.auth().authTokenValidity());
}
#endif // GSC_AUTH_COOKIE

#ifdef GSC_AUTH_PASSWORD
void Request_handler::check_password_strength(Credentials const &cred) {
  Wt::Auth::PasswordStrengthValidator psv;
  auto psv_result = psv.evaluateStrength(cred.password, cred.username, "");

  if (psv_result.isValid())
    return;

  Http_error{403} << "Please try a stronger password (" << psv_result.message()
                  << ")";
}
#endif // GSC_AUTH_PASSWORD

dbo::ptr<User> Request_handler::authenticate_by_api_key_() const {
#ifdef GSC_AUTH_API_KEY
  if (auto *cookie_val = request_.getCookieValue(api_key_name); cookie_val)
    return session_.find_by_identity<User>(api_key_identity, *cookie_val);
#endif // GSC_AUTH_API_KEY

  return nullptr;
}

dbo::ptr<User> Request_handler::authenticate_by_cookie_() const {
#ifdef GSC_AUTH_COOKIE
  std::string const *in_cookie = request_.getCookieValue(cookie_name);
  if (!in_cookie)
    return {};

  // Intercept logouts
  if (std::regex_match(path_info_, paths::Whoami::re) && method_ == "DELETE") {
    auto hash = session_.auth().tokenHashFunction()->compute(*in_cookie, "");

    dbo::Transaction transaction(session_);
    auto user = User::find_by_auth_token(session_, hash);

    if (user) {
      user.modify()->remove_auth_token(hash);
      transaction.commit();
      throw Http_status{200, "Deauthenticated"};
    } else {
      transaction.commit();
      throw Http_status{401, "You are not authenticated"};
    }
  }

  Wt::Auth::AuthTokenResult auth_result =
      session_.auth().processAuthToken(*in_cookie, session_.users());
  if (auth_result.state() != Wt::Auth::AuthTokenState::Valid)
    return {};

  set_cookie_(auth_result.newToken(), auth_result.newTokenValidity());

  return session_.users().find(auth_result.user());
#else  // GSC_AUTH_COOKIE
  return nullptr;
#endif // GSC_AUTH_COOKIE
}

dbo::ptr<User> Request_handler::authenticate_by_password_() const {
#ifdef GSC_AUTH_PASSWORD
  using Wt::Auth::PasswordResult;

  // If no authorization header is provided, give up; otherwise, parse it.
  auto auth_header = request_.headerValue("Authorization");
  if (auth_header.empty())
    return {};
  auto credentials = parse_authorization(auth_header);

  dbo::Transaction transaction(session_);

  // Look up the user by name.
  auto user = User::find_by_name(session_, credentials.username);

  // Intercept user creation requests. These are POSTs to /users, and are only
  // valid if the user doesn't exist. In that case, we create the user and
  // create and set a login cookie.
  if (std::regex_match(path_info_, paths::Users::re) && method_ == "POST") {
    if (user)
      throw Http_status{403, "User already exists"};

    check_password_strength(credentials);
    user = session_.create_user(credentials.username, credentials.password);
    create_cookie_(session_.users().find(user));

    transaction.commit();
    throw Http_status{200, "User created"};
  }

  // Otherwise, the user doesn't exist.
  if (!user)
    throw Http_status{401, "User does not exist"};

  // The user exists, so we need to validate their password.
  auto &service = session_.passwordAuth();
  auto auth_user = session_.users().find(user);
  auto verify_result = service.verifyPassword(auth_user, credentials.password);

  transaction.commit();

  switch (verify_result) {
  case PasswordResult::PasswordInvalid:
    throw Http_status{401, "Invalid password"};

  case PasswordResult::LoginThrottling:
    Http_error{401} << "Too many attempts; please wait "
                    << service.delayForNextAttempt(auth_user) << " seconds";

  case PasswordResult::PasswordValid:
    create_cookie_(auth_user);
    return user;
  }
#else  // GSC_AUTH_PASSWORD
  return nullptr;
#endif // GSC_AUTH_PASSWORD
}

dbo::ptr<User> Request_handler::authenticate_by_open_am_() const {
  if (auto auth_info =
          session_.find_from_environment<Auth_info>(false, App_environment()))
    return auth_info->user();
  else
    return nullptr;
}

} // end namespace api

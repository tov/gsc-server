#include "Request_handler.h"
#include "Http_status.h"

#include <Wt/Auth/AuthService.h>
#include <Wt/Utils.h>
#include <Wt/WDateTime.h>

#include <sstream>

namespace api {

static const std::regex authorization_pat(" *basic ([^ ]+) *",
                                          std::regex_constants::icase);

static Credentials
parse_authorization(const std::string& header_value)
{
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

Request_handler::Request_handler(Db_session& session,
                                 Wt::Http::Request const& request,
                                 Wt::Http::Response& response)
        : session_{session}
        , request_{request}
        , response_{response}
        , path_info_{request_.pathInfo()}
        , method_{request.method()}
{ }

dbo::ptr<User> Request_handler::authenticate()
{
    dbo::ptr<User> user;

    if ((user = authenticate_by_cookie_())) return user;
    if ((user = authenticate_by_password_())) return user;

    throw Http_status{401,
                      "Access to this resource requires authentication"};
}

std::unique_ptr<Resource::Base> Request_handler::parse_uri()
{
    return Resource::Base::create(method_, path_info_);
}

static std::string const cookie_name = "gsc_cookie";

void
Request_handler::set_cookie_(std::string const& value, int ttl_seconds) const
{
    auto expiry = Wt::WDateTime::currentDateTime().addSecs(ttl_seconds);

    std::ostringstream out_cookie;
    out_cookie << cookie_name << "=" << value
               << "; Expires="
               << expiry.toString("ddd, d MMM yyyy hh:mm:ss 'GMT'");

    response_.addHeader("Set-Cookie", out_cookie.str());
}

void Request_handler::create_cookie_(Wt::Auth::User const& auth_user)
{
    std::string auth_token = session_.auth().createAuthToken(auth_user);
    set_cookie_(auth_token, 60 * session_.auth().authTokenValidity());
}

void Request_handler::check_password_strength_(Credentials const& cred)
{
    Wt::Auth::PasswordStrengthValidator psv;
    auto psv_result = psv.evaluateStrength(cred.password, cred.username, "");

    if (psv_result.isValid()) return;

    std::ostringstream msg;
    msg << "Please try a stronger password (" << psv_result.message() << ")";
    throw Http_status{403, msg.str()};
}

dbo::ptr<User> Request_handler::authenticate_by_cookie_()
{
    std::string const* in_cookie = request_.getCookieValue(cookie_name);
    if (!in_cookie) return {};

    Wt::Auth::AuthTokenResult auth_result =
            session_.auth().processAuthToken(*in_cookie, session_.users());
    if (auth_result.state() != Wt::Auth::AuthTokenState::Valid)
        return {};

    set_cookie_(auth_result.newToken(), auth_result.newTokenValidity());

    return session_.users().find(auth_result.user());
}

dbo::ptr<User> Request_handler::authenticate_by_password_()
{
    using Wt::Auth::PasswordResult;

    // If no authorization header is provided, give up; otherwise, parse it.
    auto auth_header = request_.headerValue("Authorization");
    if (auth_header.empty()) return {};
    auto credentials = parse_authorization(auth_header);

    dbo::Transaction transaction(session_);

    // Look up the user by name.
    auto user = User::find_by_name(session_, credentials.username);

    // Intercept user creation requests. These are POSTs to /users, and are only
    // valid if the user doesn't exist. In that case, we create the user and
    // create and set a login cookie.
    if (path_info_ == Path::users && method_ == "POST") {
        if (user) throw Http_status{403, "User already exists"};

        check_password_strength_(credentials);
        user = session_.create_user(credentials.username, credentials.password);
        create_cookie_(session_.users().find(user));

        // Redirect request internally to GET the new user.
        method_    = "GET";
        path_info_ = "/users/" + user->name();

        return user;
    }

    // Otherwise, the user doesn't exist.
    if (!user) throw Http_status{401, "User does not exist"};

    // The user exists, so we need to validate their password.
    auto& service = session_.passwordAuth();
    auto auth_user = session_.users().find(user);

    switch (service.verifyPassword(auth_user, credentials.password)) {
        case PasswordResult::PasswordInvalid:
            throw Http_status{401, "Invalid password"};

        case PasswordResult::LoginThrottling: {
            auto seconds = service.delayForNextAttempt(auth_user);
            std::ostringstream msg;
            msg << "Too many attempts; please wait " << seconds
                << " seconds";
            throw Http_status{401, msg.str()};
        }

        case PasswordResult::PasswordValid:
            create_cookie_(auth_user);
            return user;
    }
}

} // end namespace api

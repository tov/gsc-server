#include "REST_endpoint.h"
#include "../model/auth/User.h"
#include "../model/Session.h"

#include <Wt/Auth/AuthService.h>
#include <Wt/Json/Value.h>
#include <Wt/Json/Serializer.h>
#include <Wt/Utils.h>
#include <Wt/WDateTime.h>
#include <Wt/WString.h>

#include <algorithm>
#include <regex>
#include <sstream>
#include <string>

namespace J = Wt::Json;

/*
 * HTTP STATUS CODES
 */

struct Http_status : std::runtime_error
{
public:
    Http_status(int status_code, std::string message);

    void respond(Wt::Http::Response&) const;

private:
    static const std::string exception_message_(int status_code);

    int status_code_;
    std::string message_;
};

Http_status::Http_status(int status_code, std::string message)
        : std::runtime_error{exception_message_(status_code)}
        , status_code_{status_code}
        , message_{std::move(message)}
{ }

const std::string Http_status::exception_message_(int status_code)
{
    std::ostringstream os;
    os << "HTTP Status " << status_code;
    return os.str();
}

void Http_status::respond(Wt::Http::Response& response) const
{
    std::string title;

    response.setStatus(status_code_);

    switch (status_code_) {
        case 400:
            title = "Bad request";
            break;

        case 401:
            title = "Unauthorized";
            response.addHeader("WWW-Authenticate", "Basic realm=gsc");
            break;

        case 403:
            title = "Forbidden";
            break;

        default:
            title = "Error";
    }

    response.addHeader("Content-Type", "application/json");

    J::Object json;
    json["status"]  = J::Value(status_code_);
    json["title"]   = J::Value(title);
    json["message"] = J::Value(message_);
    response.out() << J::serialize(json);
}

/*
 * URIs
 */

namespace Path {

using namespace std;

static const string users("/users");
static const regex users_1("/users/([^/])");
static const regex users_1_hws("/users/([^/])/hws");
static const regex users_1_hws_2("/users/([^/])/hws/(\\d+)");
static const regex users_1_hws_2_files("/users/([^/])/hws/(\\d+)/files");
static const regex users_1_hws_2_files_3(
        "/users/([^/])/hws/(\\d+)/files/([^/]+)");

}

/*
 * HTTP AUTHORIZATION
 */

struct Credentials
{
    std::string username;
    std::string password;
};

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

/*
 * REQUEST HANDLER
 */

class Request_handler
{
public:
    Request_handler(Db_session&,
                    Wt::Http::Request const&,
                    Wt::Http::Response&);

    dbo::ptr<User> authenticate();
    dbo::ptr<User> authenticate_by_cookie();
    dbo::ptr<User> authenticate_by_password();

private:
    void create_cookie(Wt::Auth::User const&);
    void set_cookie(std::string const& value, int ttl_seconds) const;
    void check_password_strength(Credentials const&) const;

    Db_session& session_;
    Wt::Http::Request const& request_;
    Wt::Http::Response& response_;
    std::string path_info_;
    std::string method_;
};

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

    if ( (user = authenticate_by_cookie()) ) return user;
    if ( (user = authenticate_by_password()) ) return user;

    throw Http_status{401,
                      "Access to this resource requires authentication"};
}

static std::string const cookie_name = "gsc_cookie";

void Request_handler::set_cookie(std::string const& value, int ttl_seconds) const
{
    auto expiry = Wt::WDateTime::currentDateTime().addSecs(ttl_seconds);

    std::ostringstream out_cookie;
    out_cookie << cookie_name << "=" << value
               << "; Expires="
               << expiry.toString("ddd, d MMM yyyy hh:mm:ss 'GMT'");

    response_.addHeader("Set-Cookie", out_cookie.str());
}

void Request_handler::create_cookie(Wt::Auth::User const& auth_user)
{
    std::string auth_token = session_.auth().createAuthToken(auth_user);
    set_cookie(auth_token, 60 * session_.auth().authTokenValidity());
}

void Request_handler::check_password_strength(Credentials const& cred) const
{
    Wt::Auth::PasswordStrengthValidator psv;
    auto psv_result = psv.evaluateStrength(cred.password, cred.username, "");

    if (psv_result.isValid()) return;

    std::ostringstream msg;
    msg << "Please try a stronger password (" << psv_result.message() << ")";
    throw Http_status{403, msg.str()};
}

dbo::ptr<User> Request_handler::authenticate_by_cookie()
{
    std::string const* in_cookie = request_.getCookieValue(cookie_name);
    if (!in_cookie) return {};

    Wt::Auth::AuthTokenResult auth_result =
            session_.auth().processAuthToken(*in_cookie, session_.users());
    if (auth_result.state() != Wt::Auth::AuthTokenState::Valid)
        return {};

    set_cookie(auth_result.newToken(), auth_result.newTokenValidity());

    return session_.users().find(auth_result.user());
}

dbo::ptr<User> Request_handler::authenticate_by_password()
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
        check_password_strength(credentials);
        user = session_.create_user(credentials.username, credentials.password);
        create_cookie(session_.users().find(user));
        return user;
    }

    // Otherwise, the user doesn't exist.
    if (!user) throw Http_status{401, "User does not exist"};

    // The user exists, so we need to validate their password.
    auto& service  = session_.passwordAuth();
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
            create_cookie(auth_user);
            return user;
    }
}

/*
 * REST ENDPOINT
 */

REST_endpoint::REST_endpoint(Wt::Dbo::SqlConnectionPool& pool)
        : session_{pool}
{ }

void REST_endpoint::handleRequest(const Wt::Http::Request& request,
                                  Wt::Http::Response& response)
{
    Request_handler handler(session_, request, response);

    try {
        auto user = handler.authenticate();

        std::smatch sm;

        std::cout << "*** authenticated user: " << user->name() << "\n";
        std::cout << "*** got " << request.method() << " request for: "
                << request.pathInfo() << "\n";
    } catch (const Http_status& status) {
        status.respond(response);
        return;
    }

    response.addHeader("Content-Type", "application/json");
    response.out() << "true";
}


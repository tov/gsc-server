#include "REST_endpoint.h"
#include "../model/auth/User.h"
#include "../model/Session.h"

#include <Wt/Utils.h>

#include <algorithm>
#include <regex>
#include <sstream>
#include <string>

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

    switch (status_code_) {
        case 400:
            title = "Bad request";
            break;

        case 401:
            title = "Unauthorized";
            response.addHeader("WWW-Authenticate", "Basic realm=gsc");
            break;

        default:
            title = "Error";
    }

    response.setStatus(status_code_);
    response.addHeader("Content-Type", "text/html");

    auto& out = response.out();
    out << "<html>";
    out << "<head><title>" << status_code_ << " " << title << "</title></head>";
    out << Wt::Utils::htmlEncode(message_);
    out << "</html>";
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

static const std::regex authorization_pat(" *basic ([^ ]+) *",
                                          std::regex_constants::icase);

static std::pair<std::string, std::string>
parse_authorization(const std::string& header_value)
{
    if (header_value.empty()) {
        throw Http_status{401,
                          "Access to this resource requires authentication"};
    }

    std::smatch sm;

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
    return {username, password};
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
    try {
        auto auth = parse_authorization(request.headerValue("Authorization"));

        std::string const path_info = request.pathInfo();
        std::string const method    = request.method();

        dbo::Transaction transaction(session_);

        auto user = User::find_by_name(session_, auth.first);

        if (path_info == Path::users && method == "POST") {
            if (user) throw Http_status{403, "User already exists"};
            user = session_.create_user(auth.first, auth.second);
        }

        if (!user) throw Http_status{401, "User does not exist"};

        {
            using Wt::Auth::PasswordResult;

            auto& service  = session_.passwordAuth();
            auto auth_user = session_.users().find(user);

            switch (service.verifyPassword(auth_user, auth.second)) {
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
                    break;
            }
        }

        std::smatch sm;

        std::cout << "* user is: " << user << "\n";
        std::cout << "** auth: " << auth.first << ":" << auth.second << "\n";
        std::cout << "*** got " << request.method() << " request for "
                  << request.pathInfo() << "\n";
    } catch (const Http_status& status) {
        status.respond(response);
        return;
    }

    response.addHeader("Content-Type", "application/json");
    response.out() << "true";
}


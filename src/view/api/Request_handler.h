#pragma once

#include "Resource.h"
#include "../../model/auth/User.h"
#include "../../model/Session.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

#include <memory>
#include <string>

namespace api {

struct Credentials
{
    std::string username;
    std::string password;
};

class Request_handler
{
public:
    Request_handler(Db_session&,
                    Wt::Http::Request const&,
                    Wt::Http::Response&);

    dbo::ptr<User> authenticate();
    std::unique_ptr<Resource::Base> parse_uri();

private:
    dbo::ptr<User> authenticate_by_cookie_();
    dbo::ptr<User> authenticate_by_password_();

    void create_cookie_(Wt::Auth::User const&);
    void set_cookie_(std::string const& value, int ttl_seconds) const;
    void check_password_strength_(Credentials const& cred) const;

    Db_session& session_;
    Wt::Http::Request const& request_;
    Wt::Http::Response& response_;
    std::string path_info_;
    std::string method_;
};

} // end namespace api

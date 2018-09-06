#include "Resource.h"
#include "Http_status.h"
#include "../../model/auth/User.h"
#include "../../model/Session.h"

#include <Wt/Json/Value.h>
#include <Wt/Json/Serializer.h>

#include <cstdlib>
#include <regex>
#include <string>

namespace dbo = Wt::Dbo;
namespace J = Wt::Json;

namespace api {

namespace Resource {

void Base::denied()
{
    throw Http_status{403, "You can't access that"};
}

void Base::not_found()
{
    throw Http_status{404, "The named resource does not exist"};
}

void Base::not_supported()
{
    throw Http_status{403, "The resource does not support that method"};
}

void Base::send(Wt::Http::Response& response) const
{
    if (content_type.empty()) {
        Http_status{500, "No content type"}.respond(response);
    } else {
        response.addHeader("Content-Type", content_type);
        response.out() << contents;
    }
}

class Users : public Base
{
public:
    Users() = default;

    void load(dbo::Session&, dbo::ptr<User> const&) override;

protected:
    void do_get_() override;

private:
    dbo::collection<dbo::ptr<User>> users_;
};

void Users::load(dbo::Session& session, dbo::ptr<User> const& current_user)
{
    if (current_user->role() != User::Role::Admin)
        denied();

    users_ = session.find<User>();
}

void Users::do_get_()
{
    J::Array result;

    for (auto const& user : users_)
        result.push_back(user->to_json());

    use_json(result);
}

class Users_1 : public Base
{
public:
    Users_1(std::string username)
            : username_{std::move(username)} {}

    void load(dbo::Session&, dbo::ptr<User> const&) override;

private:
    std::string username_;
    dbo::ptr<User> user_;
};

void Users_1::load(dbo::Session& session, dbo::ptr<User> const& current_user)
{
    if (current_user->name() != username_ &&
            current_user->role() != User::Role::Admin)
        denied();

    user_ = User::find_by_name(session, username_);
    if (!user_) not_found();
}

class Users_1_hws : public Base
{
public:
    Users_1_hws(std::string username)
            : username_{std::move(username)} {}

    void load(dbo::Session&, dbo::ptr<User> const&) override;

private:
    std::string username_;
};

void Users_1_hws::load(dbo::Session& session, dbo::ptr<User> const& current_user)
{

}

class Users_1_hws_2 : public Base
{
public:
    Users_1_hws_2(std::string username, int hw_number)
            : username_{std::move(username)}, hw_number_{hw_number} {}

    void load(dbo::Session&, dbo::ptr<User> const&) override;

private:
    std::string username_;
    int hw_number_;
};

void Users_1_hws_2::load(dbo::Session& session, dbo::ptr<User> const& current_user)
{

}

class Users_1_hws_2_files : public Base
{
public:
    Users_1_hws_2_files(std::string username, int hw_number)
            : username_{std::move(username)}, hw_number_{hw_number} {}

    void load(dbo::Session&, dbo::ptr<User> const&) override;

private:
    std::string username_;
    int hw_number_;
};

void Users_1_hws_2_files::load(dbo::Session& session, dbo::ptr<User> const& current_user)
{

}

class Users_1_hws_2_files_3 : public Base
{
public:
    Users_1_hws_2_files_3(std::string username,
                          int hw_number,
                          std::string filename)
            : username_{std::move(username)}, hw_number_{hw_number},
              filename_{std::move(filename)} {}

    void load(dbo::Session&, dbo::ptr<User> const&) override;

private:
    std::string username_;
    int hw_number_;
    std::string filename_;
};

void Users_1_hws_2_files_3::load(dbo::Session& session, dbo::ptr<User> const& current_user)
{

}

std::unique_ptr<Base> Base::create(std::string const& method,
                                   std::string const& path_info)
{
    auto resource = parse_(path_info);
    resource->method_ = method;
    return std::move(resource);
}

std::unique_ptr<Base> Base::parse_(std::string const& path_info)
{
    std::smatch sm;

    auto get_number = [](const auto& sm_i) {
        std::string str(sm_i.first, sm_i.second);
        return std::atoi(str.c_str());
    };

    if (path_info == Path::users) {
        return std::make_unique<Users>();
    }

    if (std::regex_match(path_info, sm, Path::users_1)) {
        std::string username(sm[1].first, sm[1].second);
        return std::make_unique<Users_1>(std::move(username));
    }

    if (std::regex_match(path_info, sm, Path::users_1_hws)) {
        std::string username(sm[1].first, sm[1].second);
        return std::make_unique<Users_1_hws>(std::move(username));
    }

    if (std::regex_match(path_info, sm, Path::users_1_hws_2)) {
        std::string username(sm[1].first, sm[1].second);
        int hw_number = get_number(sm[2]);
        return std::make_unique<Users_1_hws_2>(
                std::move(username), hw_number);
    }

    if (std::regex_match(path_info, sm, Path::users_1_hws_2_files)) {
        std::string username(sm[1].first, sm[1].second);
        int hw_number = get_number(sm[2]);
        return std::make_unique<Users_1_hws_2_files>(
                std::move(username), hw_number);
    }

    if (std::regex_match(path_info, sm, Path::users_1_hws_2_files)) {
        std::string username(sm[1].first, sm[1].second);
        int hw_number = get_number(sm[2]);
        std::string filename(sm[3].first, sm[3].second);
        return std::make_unique<Users_1_hws_2_files_3>(
                std::move(username), hw_number, std::move(filename));
    }

    not_found();
}

void Base::process()
{
    if (method_ == "DELETE")
        do_delete_();
    else if (method_ == "GET")
        do_get_();
    else if (method_ == "PATCH")
        do_patch_();
    else if (method_ == "POST")
        do_post_();
    else if (method_ == "PUT")
        do_post_();
    else not_supported();
}

void Base::do_delete_()
{
    not_supported();
}

void Base::do_get_()
{
    not_supported();
}

void Base::do_patch_()
{
    not_supported();
}

void Base::do_post_()
{
    not_supported();
}

void Base::do_put_()
{
    not_supported();
}

} // end namespace Resource

} // end namespace api

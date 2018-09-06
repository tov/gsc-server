#include "Resource.h"

#include "Http_status.h"

#include <cstdlib>
#include <regex>
#include <string>

namespace api {

namespace Resource {

class Users : public Base
{
public:
    Users() = default;
};

class Users_1 : public Base
{
public:
    Users_1(std::string username)
            : username_{std::move(username)} {}

private:
    std::string username_;
};

class Users_1_hws : public Base
{
public:
    Users_1_hws(std::string username)
            : username_{std::move(username)} {}

private:
    std::string username_;
};

class Users_1_hws_2 : public Base
{
public:
    Users_1_hws_2(std::string username, int hw_number)
            : username_{std::move(username)}, hw_number_{hw_number} {}

private:
    std::string username_;
    int hw_number_;
};

class Users_1_hws_2_files : public Base
{
public:
    Users_1_hws_2_files(std::string username, int hw_number)
            : username_{std::move(username)}, hw_number_{hw_number} {}

private:
    std::string username_;
    int hw_number_;
};

class Users_1_hws_2_files_3 : public Base
{
public:
    Users_1_hws_2_files_3(std::string username,
                          int hw_number,
                          std::string filename)
            : username_{std::move(username)}, hw_number_{hw_number},
              filename_{std::move(filename)} {}

private:
    std::string username_;
    int hw_number_;
    std::string filename_;
};

std::unique_ptr<Base> Base::parse(std::string const& path_info)
{
    std::smatch sm;

    if (path_info == Path::users) {
        return std::make_unique<Users>();
    }

    auto get_number = [](const auto& sm_i) {
        std::string str(sm_i.first, sm_i.second);
        return std::atoi(str.c_str());
    };

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

    throw Http_status{404, "The named resource does not exist"};
}

} // end namespace Resource

} // end namespace api

#pragma once

#include <string>

namespace Wt::Auth {
class User;
}

extern const std::string api_key_identity;
extern const std::string api_key_name;

void set_api_key_identity(Wt::Auth::User &);

std::string get_api_key_identity(Wt::Auth::User const &);

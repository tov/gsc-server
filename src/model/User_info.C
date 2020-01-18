#include "User_info.h"
#include "auth/User.h"
#include "../common/util.h"

DBO_INSTANTIATE_TEMPLATES(User_info)

User_info::User_info(const dbo::ptr<User>& user)
        : user_(user)
{ }

bool User_info::Map::contains(string const& key) const
{
    return map_.find(key) != map_.end();
}

void User_info::Map::remove(std::string const& key)
{
    map_.erase(key);
}

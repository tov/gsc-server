#include "User_info.h"
#include "auth/User.h"

DBO_INSTANTIATE_TEMPLATES(User_info)

User_info::User_info(const dbo::ptr<User>& user)
        : user_(user)
{

}


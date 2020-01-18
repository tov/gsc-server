#include "User_info.h"
#include "auth/User.h"
#include "../common/util.h"

DBO_INSTANTIATE_TEMPLATES(User_info)

unique_ptr<User_info>
User_info::make(dbo::ptr<User> const& user, Key const& key, Value const& value)
{
    auto result = make_unique<User_info>();
    result->user_ = user;
    result->key_ = key;
    result->value_ = value;
    return result;
}

User_info_map::User_info_map(dbo::ptr<User> user)
        : user_(move(user))
{ }

bool User_info_map::contains(string const& key) const
{
    return bool(find_(key));
}

void User_info_map::remove(std::string const& key)
{
    if (auto iter = map_.find(key); iter != map_.end()) {
        iter->second.remove();
        map_.erase(iter);
    }
}

User_info::Value const&
User_info_map::operator[](Key const& key) const {
    if (auto info = find_(key)) {
        return info->value();
    } else {
        throw Not_found(key);
    }
}

User_info::Proxy
User_info_map::operator[](User_info::Key const& key)
{
    if (auto info = find_(key)) {
        return Proxy(user_, move(key), move(info));
    } else {
        return Proxy(user_, move(key), nullptr);
    }
}

dbo::ptr<User_info>
User_info_map::find_(User_info::Key const& key) const
{
    if (auto iter = map_.find(key);
            iter != map_.end()) {
        return iter->second;
    } else {
        return nullptr;
    }
}

static std::string not_found_message(std::string const& key)
{
    return "User info key not found: " + key;
}

User_info::Not_found::Not_found(Key const& key) noexcept
        : runtime_error(not_found_message(key))
{ }

User_info::Proxy::operator Value const&() const
{
    if (value_) {
        return *value_;
    } else if (info_) {
        return info_->value();
    } else {
        throw Not_found(key_);
    }
}

User_info::Proxy& User_info::Proxy::operator=(Value const& value)
{
    if ( !has_value_(value) ) {
        value_ = value;
    }

    return *this;
}

User_info::Proxy::~Proxy()
{
    if (!value_) return;

    if (info_) {
        info_.modify()->set_value(*value_);
    } else {
        user_.session()->add(User_info::make(user_, key_, *value_));
    }
}

User_info::Proxy::Proxy(dbo::ptr<User> const& user,
                        Key key,
                        dbo::ptr<User_info> info)
        : user_(user)
        , key_(move(key))
        , info_(move(info))
{ }

bool User_info::Proxy::has_value_(Value const& value) const
{
    if (value_) {
        return *value_ == value;
    } else if (info_) {
        return info_->value() == value;
    } else {
        return false;
    }
}


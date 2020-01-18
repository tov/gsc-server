#pragma once

#include "specializations.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WDateTime.h>

#include <string>
#include <unordered_map>

namespace dbo = Wt::Dbo;

class User;

class User_info
{
public:
    class Map;
    class Proxy;

    User_info() {};
    explicit User_info(const dbo::ptr<User>&);

    dbo::ptr<User> user() const { return user_; }

    std::string const& key() const { return key_; }
    std::string const& value() const { return value_; }

    void set_value(std::string const& value) { value_ = value; }

private:
    dbo::ptr<User> user_;
    std::string    key_;
    std::string    value_;

public:
    template<class Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, user_, "user", dbo::OnDeleteCascade);
        dbo::field(a, key_, "key");
        dbo::field(a, value_, "value");
    }
};

class User_info::Map {
public:
    bool contains(std::string const& key) const;
    void remove(std::string const& key);

private:
    dbo::ptr<User> user_;
    std::unordered_map<std::string, dbo::ptr<User_info>> map_;
};

class User_info::Proxy {

};

DBO_EXTERN_TEMPLATES(User_info)

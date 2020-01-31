#pragma once

#include "specializations.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WDateTime.h>

#include <string>

namespace dbo = Wt::Dbo;

class User;

class User_info
{
public:
    User_info() {};
    User_info(const dbo::ptr<User>&);

    dbo::ptr<User> user() const { return user_; }

    std::string const& key() const { return key_; }
    std::string const& value() const { return value_; }

    void set_key(std::string const& key) { key_ = key; }
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

DBO_EXTERN_TEMPLATES(User_info)

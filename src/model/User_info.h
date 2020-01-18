#pragma once

#include "specializations.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WDateTime.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace dbo = Wt::Dbo;

class User;

// Forward declaration
class User_info_map;

class User_info
{
public:
    using Key   = std::string;
    using Value = std::string;
    using Map   = User_info_map;
    class Not_found;
    class Proxy;

    User_info() {};

    dbo::ptr<User> user() const { return user_; }

    Key const& key() const { return key_; }
    Value const& value() const { return value_; }

    void set_value(Value const& value) { value_ = value; }

private:
    static std::unique_ptr<User_info>
    make(dbo::ptr<User> const&, Key const&, Value const&);

    dbo::ptr<User> user_;
    Key key_;
    Value value_;

public:
    template<class Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, user_, "user", dbo::OnDeleteCascade);
        dbo::field(a, key_, "key");
        dbo::field(a, value_, "value");
    }
};

class User_info_map {
public:
    using Key       = User_info::Key;
    using Value     = User_info::Value;
    using Not_found = User_info::Not_found;
    using Proxy     = User_info::Proxy;

    bool contains(Key const& key) const;
    void remove(Key const& key);

    Value const& operator[](Key const&) const;
    Proxy operator[](Key const&);

private:
    dbo::ptr<User> user_;
    std::unordered_map<Key, dbo::ptr<User_info>> map_;

    explicit User_info_map(dbo::ptr<User> user);

    dbo::ptr<User_info> find_(Key const&) const;

    friend User;
    friend User_info;
};

class User_info::Not_found : public std::runtime_error
{
private:
    explicit Not_found(Key const& key) noexcept;

    friend User_info;
    friend User_info_map;
};

class User_info::Proxy {
public:
    operator Value const&() const;

    Proxy& operator=(Value const&);

    ~Proxy();

    Proxy(Proxy const&) = delete;
    Proxy& operator=(Proxy const&) = delete;

    Proxy(Proxy&&) = delete;
    Proxy& operator=(Proxy&&) = delete;

private:
    Proxy(dbo::ptr<User> const&, Key, dbo::ptr<User_info>);

    bool has_value_(Value const&) const;

    dbo::ptr<User> user_;
    Key key_;
    dbo::ptr<User_info> info_;
    std::optional<Value> value_;

    friend User_info_map;
};

DBO_EXTERN_TEMPLATES(User_info)

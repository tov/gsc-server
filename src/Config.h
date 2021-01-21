#pragma once

#include <memory>
#include <string>
#include <type_traits>

#include <Wt/Json/Object.h>
#include <Wt/Json/Value.h>

#ifndef NUSSO_XUI_URL
#  define NUSSO_XUI_URL  "https://prd-nusso.it.northwestern.edu/nusso/XUI/"
#endif

class Config const& CONFIG();

class Config
{
    friend Config const& CONFIG();

private:
    Config();

    std::unique_ptr<Wt::Json::Object> json_;

public:
    template <class T>
    using getter_t = std::conditional_t<std::is_trivial_v<T>, T, T const&>;

#pragma push_macro("FIELD")
#define FIELD(TYPE, NAME, DEFAULT) \
    public:                        \
        getter_t<TYPE> NAME() const { return NAME##_; } \
    private:              \
        void set_##NAME##_() \
        { NAME##_ = get<TYPE>(#NAME, DEFAULT); } \
        TYPE NAME##_

    FIELD(float,       daily_discount,          0.01);
    FIELD(float,       reward_for_no,           0.5);
    FIELD(bool,        auto_create_accounts,    true);
    FIELD(bool,        show_queries,            false);
    FIELD(bool,        display_file_quota,      true);
    FIELD(std::string, instance_class,          "instance-default");
    FIELD(std::string, instance_name,           "gsc");
    FIELD(std::string, gsc_base_url,            "http://localhost:9090/");
    FIELD(std::string, postgres_conninfo,       "dbname=gsc");
    FIELD(std::string, server_time_zone,        "America/Chicago");
    FIELD(std::string, websso_login_url,        NUSSO_XUI_URL "#login");
    FIELD(std::string, websso_logout_url,       NUSSO_XUI_URL "#logout");

#pragma pop_macro("FIELD")

public:
    Wt::Json::Value
    const& get(std::string const& key) const;

    template <class VALUE>
    VALUE
    get(std::string const& key, VALUE const& otherwise) const;
};

template<class VALUE>
VALUE
Config::get(const std::string& key, const VALUE& otherwise) const
{
    return get(key).orIfNull(otherwise);
}

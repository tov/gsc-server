#pragma once

#include <Wt/WApplication>

class Navigate
{
public:
    Navigate(const std::string& uri) : uri_(uri) {}
    Navigate(const std::string&& uri) : uri_(std::move(uri)) {}

    static void to(const std::string& uri)
    {
        Wt::WApplication::instance()->setInternalPath(uri, true);
    }

    template<typename... T>
    void operator()(T&&...)
    {
        to(uri_);
    }

private:
    std::string uri_;
};


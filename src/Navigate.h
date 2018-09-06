#pragma once

#include <Wt/WApplication.h>

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

template <>
struct Wt::Signals::Impl::function_traits<Navigate>
{
    static const size_t argCount = 0;
    using function = std::function<void()>;
};


#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

namespace Wt {
    namespace Http {
        class Response;
    }
}

namespace api {

struct Http_status : std::runtime_error
{
public:
    Http_status(int status_code, std::string message);

    void respond(Wt::Http::Response&) const;

private:
    static const std::string exception_message_(int status_code);

    int status_code_;
    std::string message_;
};

struct Http_error
{
public:
    Http_error(int status_code);

    template <class T>
    Http_error& operator<<(T const& arg)
    {
        message_ << arg;
        return *this;
    }

    ~Http_error [[noreturn]]() noexcept(false);

private:
    int status_code_;
    std::ostringstream message_;
};

} // end namespace api

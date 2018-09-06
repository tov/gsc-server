#pragma once

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

} // end namespace api

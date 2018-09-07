#pragma once

#include <Wt/Json/Value.h>

#include <istream>
#include <string>

namespace api {

class Request_body
{
public:
    explicit Request_body(Wt::Http::Request const& request)
            : in_{&request.in()}, size_{request.contentLength()} {}

    int size() const { return size_; };

    std::string read_string()&&;
    Wt::Json::Value read_json()&&;

private:
    std::istream* in_;
    int size_;
};

} // end namespace api

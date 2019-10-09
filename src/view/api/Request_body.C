#include "Request_body.h"
#include "Http_status.h"
#include "../../model/File_data.h"

#include <Wt/Json/Parser.h>

namespace api {

Wt::Json::Value Request_body::read_json()&&
{
    std::string buffer{std::move(*this).read_string()};
    Wt::Json::ParseError error;
    Wt::Json::Value result;

    if (Wt::Json::parse(buffer, result, error))
        return result;

    throw Http_status{400, "Request body expected to be JSON"};
}

Bytes Request_body::read_bytes() &&
{
    Bytes result{in_, size_};
    size_ = 0;
    return result;
}

std::string Request_body::read_string()&&
{
    std::string result;
    result.reserve(size_);
    result.assign(std::istreambuf_iterator<char>(in_), {});

    size_ = 0;

    return result;
}

} // end namespace api

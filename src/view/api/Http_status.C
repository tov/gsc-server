#include "Http_status.h"

#include <Wt/Json/Value.h>
#include <Wt/Json/Serializer.h>

#include <sstream>

namespace J = Wt::Json;

namespace api {

Http_status::Http_status(int status_code, std::string message)
        : std::runtime_error{exception_message_(status_code)},
          status_code_{status_code}, message_{std::move(message)} {}

const std::string Http_status::exception_message_(int status_code)
{
    std::ostringstream os;
    os << "HTTP Status " << status_code;
    return os.str();
}

void Http_status::respond(Wt::Http::Response& response) const
{
    std::string title;

    response.setStatus(status_code_);

    switch (status_code_) {
        case 400:
            title = "Bad request";
            break;

        case 401:
            title = "Unauthorized";
            response.addHeader("WWW-Authenticate", "Basic realm=gsc");
            break;

        case 403:
            title = "Forbidden";
            break;

        case 404:
            title = "Not found";
            break;

        default:
            title = "Error";
    }

    response.addHeader("Content-Type", "application/json");

    J::Object json;
    json["status"] = J::Value(status_code_);
    json["title"] = J::Value(title);
    json["message"] = J::Value(message_);
    response.out() << J::serialize(json);
}

} // end namespace api

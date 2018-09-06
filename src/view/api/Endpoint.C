#include "Endpoint.h"
#include "Http_status.h"
#include "Path.h"
#include "Request_handler.h"
#include "Resource.h"

namespace api {

Endpoint::Endpoint(Wt::Dbo::SqlConnectionPool& pool)
        : session_{pool} {}

void Endpoint::handleRequest(const Wt::Http::Request& request,
                             Wt::Http::Response& response)
{
    Request_handler handler(session_, request, response);

    try {
        auto user = handler.authenticate();
        auto uri = handler.parse_uri();

        std::cout << "*** authenticated user: " << user->name() << "\n";
        std::cout << "*** got " << request.method() << " request for: "
                  << request.pathInfo() << "\n";
    } catch (const Http_status& status) {
        status.respond(response);
        return;
    }

    response.addHeader("Content-Type", "application/json");
    response.out() << "true";
}

} // end namespace api

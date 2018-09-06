#include "Endpoint.h"
#include "Http_status.h"
#include "Path.h"
#include "Request_handler.h"
#include "Resource.h"

#include <memory>

namespace api {

Endpoint::Endpoint(Wt::Dbo::SqlConnectionPool& pool)
        : session_{pool} {}

void Endpoint::handleRequest(const Wt::Http::Request& request,
                             Wt::Http::Response& response)
{
    Request_handler handler(session_, request, response);

    std::unique_ptr<Resource::Base> resource;

    try {
        auto current_user = handler.authenticate();
        std::cout << "*** authenticated user: " << current_user->name() << "\n";
        std::cout << "*** got " << request.method() << " request for: "
                  << request.pathInfo() << "\n";

        resource = handler.parse_uri();

        Wt::Dbo::Transaction transaction(session_);
        resource->load(session_, current_user);
        resource->process();

    } catch (const Http_status& status) {
        status.respond(response);
        return;
    }

    resource->send(response);
}

} // end namespace api

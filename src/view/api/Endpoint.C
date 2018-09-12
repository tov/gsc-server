#include "Endpoint.h"
#include "Http_status.h"
#include "Path.h"
#include "Request_handler.h"
#include "Resource.h"
#include "../../common/date_time.h"

#include <memory>

namespace api {

Endpoint::Endpoint(Wt::Dbo::SqlConnectionPool& pool)
        : session_{pool}
        , locale_{Wt::WLocale::currentLocale()}
{
    set_time_zone(locale_);
}

void Endpoint::handleRequest(const Wt::Http::Request& request,
                             Wt::Http::Response& response)
{
    Wt::WLocale::setCurrentLocale(locale_);

    std::unique_ptr<Resource::Base> resource;

    try {
        Request_handler handler(session_, request, response);
        auto current_user = handler.authenticate();
        resource = handler.parse_uri();

        Wt::Dbo::Transaction transaction(session_);
        Resource::Base::Context context{session_, current_user};
        resource->load(context);
        resource->process(request, context);

    } catch (const Http_status& status) {
        status.respond(response);
        return;
    }

    resource->send(response);
}

} // end namespace api

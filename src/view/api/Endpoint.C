#include "Endpoint.h"
#include "Http_status.h"
#include "paths.h"
#include "Request_handler.h"
#include "resources.h"
#include "../../common/date_time.h"

#include <memory>

namespace api {

Endpoint::Endpoint(Wt::Dbo::SqlConnectionPool& pool)
        : pool_{pool}
        , locale_{Wt::WLocale::currentLocale()}
{
    set_time_zone(locale_);
}

void Endpoint::handleRequest(const Wt::Http::Request& request,
                             Wt::Http::Response& response)
{
    Wt::WLocale::setCurrentLocale(locale_);
    Db_session session(pool_);

    std::unique_ptr<resources::Resource> resource;

    try {
        Request_handler handler(session, request, response);
        auto current_user = handler.authenticate();
        resource = handler.parse_uri();

        Wt::Dbo::Transaction transaction(session);
        resources::Resource::Context context{session, current_user};
        resource->load(context);
        resource->process(request, context);

    } catch (const Http_status& status) {
        status.respond(response);
        return;
    }

    resource->send(response);
}

} // end namespace api

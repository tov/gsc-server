#pragma once

#include "../../model/Session.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WResource.h>

namespace api {

class Endpoint : public Wt::WResource
{
public:
    Endpoint(Wt::Dbo::SqlConnectionPool& pool);

    void handleRequest(Wt::Http::Request const&,
                       Wt::Http::Response&) override;

private:
    Db_session session_;
};

} // end namespace api

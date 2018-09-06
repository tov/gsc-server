#pragma once

#include "../model/Session.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WResource.h>

class REST_endpoint : public Wt::WResource
{
public:
    REST_endpoint(Wt::Dbo::SqlConnectionPool& pool);

    void handleRequest(Wt::Http::Request const&,
                       Wt::Http::Response&) override;

private:
    Db_session session_;
};

#pragma once

#include "../../Session.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WLocale.h>
#include <Wt/WResource.h>

namespace api {

class Endpoint : public Wt::WResource
{
public:
    Endpoint(Wt::Dbo::SqlConnectionPool& pool);

    void handleRequest(Wt::Http::Request const&,
                       Wt::Http::Response&) override;

private:
    Wt::Dbo::SqlConnectionPool& pool_;
    Wt::WLocale locale_;
};

} // end namespace api

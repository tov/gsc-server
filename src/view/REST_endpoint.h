#pragma once

#include "../model/Session.h"

#include <Wt/WResource.h>

class REST_endpoint : public Wt::WResource
{
public:
    REST_endpoint(Wt::Dbo::SqlConnectionPool& pool);

    void handleRequest(const Wt::Http::Request&,
                       Wt::Http::Response&) override;

private:
    Db_session session_;
};

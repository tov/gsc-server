#pragma once

#include <Wt/WResource.h>

class REST_endpoint : public Wt::WResource
{
public:
    REST_endpoint(Wt::Dbo::SqlConnectionPool& pool);

    void handleRequest(const Wt::Http::Request&,
                       Wt::Http::Response&) override;

private:
    Wt::Dbo::SqlConnectionPool& pool_;
};

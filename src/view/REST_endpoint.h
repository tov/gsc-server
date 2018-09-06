#pragma once

#include <Wt/WResource>

class REST_endpoint : public Wt::WResource
{
public:
    void handleRequest(const Wt::Http::Request&,
                       Wt::Http::Response&) override;
};

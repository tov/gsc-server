#pragma once

#include "../model/util/Bytes.h"

#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/Http/Response.h>
#include <Wt/WResource.h>

#include <string>

class Session;

struct Resource_response
{
    std::string content_type;
    Bytes       contents;

    void send(Wt::Http::Response&) const;
};

class Resource_base : public Wt::WResource
{
public:
    using Response = Resource_response;

    explicit Resource_base(Wt::Dbo::SqlConnectionPool&);

protected:
    virtual void
    handleRequest(Wt::Http::Request const&, Response&, Session&) = 0;

public:
    void
    handleRequest(Wt::Http::Request const&, Wt::Http::Response&) final;

private:
    Wt::Dbo::SqlConnectionPool& pool_;
};


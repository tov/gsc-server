#pragma once

#include "../../Session.h"
#include "../Resource_base.h"

#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>

namespace api {

class Resource : public Resource_base {
public:
  using Resource_base::Resource_base;

protected:
  void handleRequest(Wt::Http::Request const &request, Response &rsrcrsp,
                     Session &session) override;
};

} // end namespace api

#include "../../common/format.h"
#include "../../common/paths.h"
#include "../Http_status.h"
#include "Request_handler.h"
#include "Resource.h"
#include "resources.h"

#include <memory>

namespace api {

void Resource::handleRequest(Wt::Http::Request const &request,
                             Response &response, Session &session) {
  Request_handler handler(session, request);
  auto current_user = handler.authenticate();
  auto resource = handler.parse_uri();
  return resource->process(request, response, {session, current_user});
}

} // end namespace api

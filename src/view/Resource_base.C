#include "../Session.h"
#include "../common/util.h"
#include "Http_status.h"
#include "Resource_base.h"

#include <Wt/Http/Response.h>

Resource_base::Resource_base(dbo::SqlConnectionPool &pool) : pool_(pool) {}

void Resource_base::handleRequest(Http::Request const &request,
                                  Http::Response &http_response) {
  Session session(pool_);
  Response response;

  try {
    dbo::Transaction transaction(session);
    handleRequest(request, response, session);
  } catch (Http_status const &status) {
    status.respond(http_response);
    return;
  } catch (std::exception &e) {
    // TODO: Make this say more?
    Http_status(500, e.what()).respond(http_response);
    return;
  }

  response.send(http_response);
}

void Resource_response::send(Http::Response &response) const {
  if (content_type.empty()) {
    Http_status{500, "No content type"}.respond(response);
  } else {
    response.setMimeType(content_type);
    response.out().write((const char *)contents.data(), contents.size());
  }
}

#include "../../Session.h"
#include "../../common/util.h"
#include "../../model/File_data.h"
#include "../../model/File_meta.h"
#include "../../model/Submission.h"
#include "../../model/auth/User.h"
#include "../Http_status.h"
#include "Resource.h"

#include <regex>

namespace files {

namespace {

void not_found(string const &message = "Resource not found") {
  throw Http_status(404, message);
}

void denied(string const &message = "Can't touch this") {
  throw Http_status(403, message);
}

const regex pat("(?:/files)?/([^/]+)/(\\d+)/([^/]+)");

struct File_request {
  string owner_name;
  int assignment;
  string filename;

  explicit File_request(string const &path_info);
};

File_request::File_request(string const &path_info) {
  smatch sm;
  if (!regex_match(path_info, sm, pat))
    not_found("Bad URI format: " + path_info);

  owner_name = string(sm[1].first, sm[1].second);
  assignment = atoi(&*sm[2].first);
  filename = string(sm[3].first, sm[3].second);
}

} // namespace

void Resource::handleRequest(Wt::Http::Request const &request,
                             Response &response, Session &session) {
  if (!session.authenticate_from_environment(Req_environment(request)))
    denied("Could not authenticate");

  File_request parsed(request.pathInfo());

  auto owner = User::find_by_name(session, parsed.owner_name);
  if (!owner)
    not_found("No such user");

  auto submission = Submission::find_by_assignment_number_and_user(
      session, parsed.assignment, owner);
  if (!submission)
    not_found("No such submission");

  if (!submission->can_view(session.user()))
    denied("That isn't yours");

  auto file_meta = submission->find_file_by_name(parsed.filename);
  if (!file_meta)
    not_found("No such file");

  auto file_data = file_meta->file_data().lock();
  if (!file_data)
    not_found("File data absent");

  response.content_type = file_meta->media_type();
  response.contents = file_data->contents();
}

} // end namespace files

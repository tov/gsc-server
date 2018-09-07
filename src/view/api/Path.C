#include "Path.h"

namespace api {

namespace Path {

using namespace std;

const string users("/users");
const regex users_1("/users/([^/]+)");
const regex users_1_hws("/users/([^/]+)/hws");
const regex users_1_hws_2("/users/([^/]+)/hws/(\\d+)");
const regex users_1_hws_2_files("/users/([^/]+)/hws/(\\d+)/files");
const regex users_1_hws_2_files_3(
        "/users/([^/]+)/hws/(\\d+)/files/([^/]+)");
const regex submissions_1("/submissions/(\\d+)");
const regex submissions_1_files("/submissions/(\\d+)/files");
const regex submissions_1_files_2("/submissions/(\\d+)/files/([^/]+)");

} // end namespace Path

} // end namespce api

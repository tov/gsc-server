#include "Path.h"

namespace api {

namespace Path {

using namespace std;

const regex users("/+users");
const regex users_1("/+users/+([^/]+)");
const regex users_1_submissions("/+users/+([^/]+)/+submissions");
const regex submissions_1("/+submissions/+(\\d+)");
const regex submissions_1_files("/+submissions/+(\\d+)/+files");
const regex submissions_1_files_2("/+submissions/+(\\d+)/+files/+([^/]+)");
const regex submissions_hw1("/+submissions/+hw(\\d+)");

} // end namespace Path

} // end namespce api

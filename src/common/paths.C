#include "paths.h"

#include <Wt/Utils.h>

#include <cstdlib>
#include <sstream>

namespace api {

namespace paths {

using namespace std;

static string extract_string(smatch::const_reference match) {
  return {match.first, match.second};
}

static int extract_int(smatch::const_reference match) {
  auto str = extract_string(match);
  return atoi(str.c_str());
}

void Uri_base::format(ostream &o) const { o << "/api"; }

string Uri_base::to_string() const {
  ostringstream o;
  format(o);
  return o.str();
}

Uri_base::operator string() const { return to_string(); }

const regex Grades_csv::re("/+grades\\.csv");

void Grades_csv::format(ostream &o) const {
  Uri_base::format(o);
  o << "/grades.csv";
}

const regex Users::re("/+users");

void Users::format(ostream &o) const {
  Uri_base::format(o);
  o << "/users";
}

const regex Users_1::re("/+users/+([^/]+)");

Users_1::Users_1(smatch const &sm) : Users_1{extract_string(sm[1])} {}

void Users_1::format(ostream &o) const {
  Users::format(o);
  o << "/" << name;
}

const regex Users_1_submissions::re("/+users/+([^/]+)/+submissions");

void Users_1_submissions::format(ostream &o) const {
  Users_1::format(o);
  o << "/submissions";
}

const regex Submissions_1::re("/+submissions/+(\\d+)");

Submissions_1::Submissions_1(smatch const &sm)
    : Submissions_1{extract_int(sm[1])} {}

void Submissions_1::format(ostream &o) const {
  Uri_base::format(o);
  o << "/submissions/" << submission_id;
}

const regex Submissions_1_files::re("/+submissions/+(\\d+)/+files");

void Submissions_1_files::format(ostream &o) const {
  Submissions_1::format(o);
  o << "/files";
}

const regex Submissions_1_files_2::re("/+submissions/+(\\d+)/+files/+([^/]+)");

Submissions_1_files_2::Submissions_1_files_2(smatch const &sm)
    : Submissions_1_files{sm}, filename{extract_string(sm[2])} {}

void Submissions_1_files_2::format(ostream &o) const {
  Submissions_1_files::format(o);
  o << "/" << Wt::Utils::urlEncode(filename);
}

const regex Submissions_1_evals::re("/+submissions/+(\\d+)/+evals");

void Submissions_1_evals::format(ostream &o) const {
  Submissions_1::format(o);
  o << "/evals";
}

const regex Submissions_1_evals_2::re("/+submissions/+(\\d+)/+evals/+(\\d+)");

Submissions_1_evals_2::Submissions_1_evals_2(smatch const &sm)
    : Submissions_1_evals{sm}, sequence{extract_int(sm[2])} {}

void Submissions_1_evals_2::format(ostream &o) const {
  Submissions_1_evals::format(o);
  o << "/" << sequence;
}

const regex Submissions_1_evals_2_self::re(
    "/+submissions/+(\\d+)/+evals/+(\\d+)/+self");

void Submissions_1_evals_2_self::format(std::ostream &o) const {
  Submissions_1_evals_2::format(o);
  o << "/self";
}

const regex Submissions_1_evals_2_grader::re(
    "/+submissions/+(\\d+)/+evals/+(\\d+)/+grader");

void Submissions_1_evals_2_grader::format(std::ostream &o) const {
  Submissions_1_evals_2::format(o);
  o << "/grader";
}

const regex Submissions_hw1::re("/+submissions/+hw(\\d+)");

Submissions_hw1::Submissions_hw1(smatch const &sm)
    : Submissions_hw1{extract_int(sm[1])} {}

void Submissions_hw1::format(ostream &o) const {
  Uri_base::format(o);
  o << "/submissions/hw" << assignment_number;
}

const regex Whoami::re("/+whoami");

void Whoami::format(ostream &o) const {
  Uri_base::format(o);
  o << "/whoami";
}

} // end namespace paths

} // namespace api

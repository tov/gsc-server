#include "../common/format.h"
#include "../common/paths.h"
#include "../common/util.h"
#include "Assignment.h"
#include "Eval_item.h"
#include "Grader_eval.h"
#include "Self_eval.h"
#include "Submission.h"

#include "Permalink.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Json/Array.h>
#include <Wt/Json/Value.h>
#include <Wt/WDateTime.h>

#include <algorithm>

namespace J = Wt::Json;

using namespace std;

DBO_INSTANTIATE_TEMPLATES(Self_eval)

static double default_score(Eval_item::Type type) {
  if (type == Eval_item::Type::Boolean)
    return 0;
  else
    return 1;
}

dbo::ptr<Self_eval> Self_eval::create(const dbo::ptr<Eval_item> &eval_item,
                                      const dbo::ptr<Submission> &submission) {
  auto &session = *eval_item.session();

  auto result =
      session.add(unique_ptr<Self_eval>{new Self_eval{eval_item, submission}});

  if (eval_item->is_graded_automatically()) {
    session.addNew<Grader_eval>(result, submission->user1(), 1);
  }

  submission.modify()->touch();

  return result;
}

Self_eval::Self_eval(const dbo::ptr<Eval_item> &eval_item,
                     const dbo::ptr<Submission> &submission)
    : Abstract_evaluation(default_score(eval_item->type())),
      eval_item_(eval_item), submission_(submission),
      permalink_(create_permalink(permalink_size)) {}

bool Self_eval::frozen() const {
  return grader_eval() && grader_eval()->score() == 1;
}

bool Self_eval::frozen_score() const {
  return grader_eval() && eval_item()->type() == Eval_item::Type::Boolean &&
         score() == 1;
}

std::string Self_eval::eval_url() const {
  std::ostringstream fmt;

  fmt << "/~" << submission()->user1()->name();
  fmt << "/hw/" << submission()->assignment()->number();
  fmt << "/eval/" << eval_item()->sequence();

  return fmt.str();
}

std::string Self_eval::grade_url() const { return "/grade/" + permalink(); }

dbo::ptr<Self_eval> Self_eval::find_by_permalink(dbo::Session &dbo,
                                                 std::string const &permalink) {
  return dbo.find<Self_eval>().where("permalink = ?").bind(permalink);
}

string Self_eval::find_ungraded_permalink(dbo::Session &dbo,
                                          const dbo::ptr<User> &user) {
  dbo.execute("DELETE FROM grader_eval"
              " WHERE status = ?"
              "   AND time_stamp < NOW() AT TIME ZONE 'UTC' - INTERVAL '1 hr'")
      .bind((int)Grader_eval::Status::editing);

  std::string current =
      dbo.query<std::string>(
             "SELECT s.permalink"
             "  FROM self_eval s"
             " INNER JOIN grader_eval g ON g.self_eval_id = s.id"
             " WHERE g.grader_id = ?"
             "   AND g.status = ?"
             " LIMIT 1")
          .bind(user.id())
          .bind((int)Grader_eval::Status::editing)
          .resultValue();
  if (!current.empty())
    return current;

  return dbo
      .query<std::string>(
          "SELECT s.permalink"
          " FROM self_eval s"
          " INNER JOIN submission b ON b.id = s.submission_id"
          " INNER JOIN eval_item e ON s.eval_item_id = e.id"
          " INNER JOIN assignment a ON e.assignment_number = a.number"
          " LEFT OUTER JOIN grader_eval g ON g.self_eval_id = s.id"
          " WHERE g.self_eval_id IS NULL"
          "   AND a.due_date < utc_now()"
          "   AND b.due_date < utc_now()"
          "   AND a.eval_date < utc_now()"
          "   AND b.eval_date < utc_now()"
          " ORDER BY a.number, e.sequence, random()"
          " LIMIT 1")
      .resultValue();
}

dbo::collection<dbo::ptr<Self_eval>>
Self_eval::find_with_grade_status(Grader_eval::Status status,
                                  dbo::Session &dbo) {
  return dbo
      .query<dbo::ptr<Self_eval>>(
          "SELECT s"
          " FROM self_eval s"
          " INNER JOIN eval_item e ON s.eval_item_id = e.id"
          " INNER JOIN assignment a ON e.assignment_number = a.number"
          " INNER JOIN grader_eval g ON s.id = g.self_eval_id"
          " WHERE g.status = ?"
          " ORDER BY a.number, e.sequence")
      .bind((int)status)
      .resultList();
}

std::string Self_eval::rest_uri() const {
  return api::paths::Submissions_1_evals_2_self(submission()->id(),
                                                eval_item()->sequence());
}

J::Object Self_eval::to_json(Viewing_context const &) const {
  J::Object result = Abstract_evaluation::to_json();

  result["uri"] = J::Value(rest_uri());
  result["permalink"] = J::Value(permalink());

  return result;
}

static string submission_owner_string(Submission const &submission,
                                      Viewing_context const &cxt) {
  if (cxt.viewer == submission.user1() || cxt.viewer == submission.user2())
    return "You";

  switch (cxt.viewer->role()) {
  case User::Role::Student:
    return "Other student";
  case User::Role::Grader:
    return "Student";
  case User::Role::Admin:
    return submission.owner_string();
  }
}

Score_owner Self_eval::score_owner(Viewing_context const &cxt) const {
  WString score =
      cxt.viewer->role() != User::Role::Grader ? plain_score_string() : "[***]";
  WString owner = submission_owner_string(*submission(), cxt);
  return {score, owner};
}

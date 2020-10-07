#pragma once

#include "../common/stringify.h"
#include "specializations.h"

#include "Abstract_evaluation.h"
#include "auth/User.h"

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Json/Object.h>
#include <Wt/WDateTime.h>

#include <string>

namespace dbo = Wt::Dbo;

class User;
class Self_eval;
class Session;
class Submission;

class Grader_eval : public Abstract_evaluation {
public:
  Grader_eval() {}

  Grader_eval(const dbo::ptr<Self_eval> &, const dbo::ptr<User> &grader);

  Grader_eval(const dbo::ptr<Self_eval> &, const dbo::ptr<User> &grader,
              double score, const std::string &explanation = "");

  enum class Status : int {
    editing,
    held_back,
    ready,
  };

  Status status() const { return static_cast<Status>(status_); }
  void set_status(Status status) { status_ = static_cast<int>(status); }
  const dbo::ptr<Self_eval> &self_eval() const { return self_eval_; }
  const dbo::ptr<User> &grader() const { return grader_; }
  void set_grader(const dbo::ptr<User> &grader) { grader_ = grader; }

  bool is_ready() const { return status() == Status::ready; }

  Score_owner score_owner(Viewing_context const &cxt) const override;

  const Wt::Dbo::ptr<Eval_item> &eval_item() const override;
  const Wt::Dbo::ptr<Submission> &submission() const override;

  bool can_view(dbo::ptr<User> const &) const;
  void check_can_view(dbo::ptr<User> const &) const;

  std::string rest_uri() const;
  Wt::Json::Object to_json(Viewing_context const &cxt) const;

  static dbo::ptr<Grader_eval> get_for(const dbo::ptr<Self_eval> &, Session &);

protected:
  std::string plain_score_string() const override;

private:
  dbo::ptr<Self_eval> self_eval_;
  dbo::ptr<User> grader_;
  int status_;

  bool can_see_score_(Viewing_context const &cxt) const;

public:
  template <typename Action> void persist(Action &a) {
    dbo::belongsTo(a, self_eval_, "self_eval", dbo::OnDeleteCascade);
    dbo::belongsTo(a, grader_, "grader", dbo::OnDeleteSetNull);
    dbo::field(a, status_, "status");
    Abstract_evaluation::persist_(a);
  }
};

template <> struct Enum<Grader_eval::Status> {
  static char const *show(Grader_eval::Status);
  static Grader_eval::Status read(std::string_view);
};

DBO_EXTERN_TEMPLATES(Grader_eval)

#pragma once

#include "../common/stringify.h"
#include "specializations.h"

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include <stdexcept>
#include <string_view>

namespace dbo = Wt::Dbo;

class Assignment;
class Submission;
class User;

class Partner_request {
public:
  enum class Status {
    outgoing,
    incoming,
    accepted,
    canceled,
  };

  Partner_request() = default;
  Partner_request(const dbo::ptr<User> &requestor,
                  const dbo::ptr<User> &requestee,
                  const dbo::ptr<Assignment> &assignment);

  static Wt::Dbo::ptr<Partner_request>
  create(Wt::Dbo::Session &session, const dbo::ptr<User> &requestor,
         const dbo::ptr<User> &requestee,
         const dbo::ptr<Assignment> &assignment, std::ostream &message);

  static Wt::Dbo::ptr<Submission> confirm(Wt::Dbo::Session &,
                                          Wt::Dbo::ptr<User>,
                                          Wt::Dbo::ptr<User>,
                                          Wt::Dbo::ptr<Assignment> const &);

  // Returns the pointer to the joint submission on success.
  dbo::ptr<Submission> confirm(Wt::Dbo::Session &session) const;

  const dbo::ptr<User> &requestor() const { return requestor_; }
  const dbo::ptr<User> &requestee() const { return requestee_; }
  const dbo::ptr<Assignment> &assignment() const { return assignment_; }

  // Can this Partner_request still be acted on?
  bool is_active(Wt::Dbo::Session &) const;

  static dbo::ptr<Partner_request> find_by_requestor_and_requestee(
      Wt::Dbo::Session &session, const dbo::ptr<User> &requestor,
      const dbo::ptr<User> &requestee, const dbo::ptr<Assignment> &assignment);

  static dbo::collection<dbo::ptr<Partner_request>>
  find_by_requestor(Wt::Dbo::Session &session, const dbo::ptr<User> &requestor);

  static dbo::collection<dbo::ptr<Partner_request>>
  find_by_requestee(Wt::Dbo::Session &session, const dbo::ptr<User> &requestee);

  static dbo::ptr<Partner_request>
  find_by_requestor_and_assignment(Wt::Dbo::Session &, const dbo::ptr<User> &,
                                   const dbo::ptr<Assignment> &);

  static dbo::collection<dbo::ptr<Partner_request>>
  find_by_requestee_and_assignment(Wt::Dbo::Session &, const dbo::ptr<User> &,
                                   const dbo::ptr<Assignment> &);

  static void delete_requests(Wt::Dbo::Session &session,
                              const dbo::ptr<User> &user,
                              const dbo::ptr<Assignment> &assignment);

private:
  dbo::ptr<User> requestor_;
  dbo::ptr<User> requestee_;
  dbo::ptr<Assignment> assignment_;

public:
  template <typename Action> void persist(Action &a) {
    dbo::belongsTo(a, requestor_, "requestor", dbo::OnDeleteCascade);
    dbo::belongsTo(a, requestee_, "requestee", dbo::OnDeleteCascade);
    dbo::belongsTo(a, assignment_, "assignment", dbo::OnDeleteCascade);
  }
};

bool operator<(Partner_request const &, Partner_request const &);

template <> struct Enum<Partner_request::Status> {
  static char const *show(Partner_request::Status);
  static Partner_request::Status read(const char *);
};

DBO_EXTERN_TEMPLATES(Partner_request)

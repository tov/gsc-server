#include "Assignment.h"
#include "Partner_request.h"
#include "Submission.h"
#include "auth/User.h"

#include <Wt/Dbo/Impl.h>

#include <regex>

DBO_INSTANTIATE_TEMPLATES(Partner_request)

namespace s {
namespace {

struct user {
  dbo::ptr<User> const &data;
  const char *const before = "User";
};

std::ostream &operator<<(std::ostream &o, user u) {
  return o << u.before << " ‘" << u.data->name() << "’";
}

struct hw {
  dbo::ptr<Assignment> const &data;
  const char *const before = "hw";
};

std::ostream &operator<<(std::ostream &o, hw h) {
  return o << h.before << h.data->number();
}

} // namespace
} // namespace s

dbo::ptr<Partner_request>
Partner_request::create(dbo::Session &session, const dbo::ptr<User> &requestor,
                        const dbo::ptr<User> &requestee,
                        const dbo::ptr<Assignment> &assignment,
                        std::ostream &message) {
  if (!assignment->partner()) {
    message << s::hw{assignment, "HW"} << " does not allow partners.";
    return {};
  }

  if (requestor == requestee) {
    message << "You cannot be your own partner.";
    return {};
  }

  auto my_submission =
      Submission::find_by_assignment_and_user(session, assignment, requestor);

  if (auto user2 = my_submission->user2()) {
    if (user2 == requestee) {
      message << s::user{requestee, "User"} << " is already your partner for "
              << s::hw{assignment} << ".";
    } else {
      message << "You already have a different partner for "
              << s::hw{assignment} << ".";
    }

    return {};
  }

  if (Partner_request::find_by_requestor_and_requestee(session, requestor,
                                                       requestee, assignment)) {
    message << "You already have an outgoing partner request to "
            << s::user{requestee} << " for " << s::hw{assignment} << ".";
    return {};
  }

  if (Partner_request::find_by_requestor_and_requestee(session, requestee,
                                                       requestor, assignment)) {
    message << s::user{requestee, "User"}
            << " has already sent you a partner request for "
            << s::hw{assignment} << "; you should accept it.";
    return {};
  }

  if (!my_submission->can_submit(requestor)) {
    message << "You cannot modify your " << s::hw{assignment}
            << " submission now.";
    return {};
  }

  if (requestee->role() == User::Role::Admin) {
    message << "You should be so lucky.";
    return {};
  }

  if (requestee->role() != User::Role::Student) {
    message << s::user{requestee, "User"} << " is not a student.";
    return {};
  }

  auto other_submission =
      Submission::find_by_assignment_and_user(session, assignment, requestee);

  if (other_submission->user2() || !other_submission->can_submit(requestee)) {
    message << s::user{requestee, "User"} << " is not available.";
    return {};
  }

  if (Partner_request::find_by_requestor_and_assignment(session, requestor,
                                                        assignment)) {
    message << "You already have an outgoing partner request for "
            << s::hw{assignment} << ".";
    return {};
  }

  return session.addNew<Partner_request>(requestor, requestee, assignment);
}

dbo::ptr<Submission> Partner_request::confirm(dbo::Session &session) const {
  // Confirm that the request hasn't been cancelled
  auto self = Partner_request::find_by_requestor_and_requestee(
      session, requestor_, requestee_, assignment_);
  if (!self)
    throw Withdrawn_partner_request_error{};

  return confirm(session, requestor_, requestee_, assignment_);
}

dbo::ptr<Partner_request> Partner_request::find_by_requestor_and_requestee(
    dbo::Session &session, const dbo::ptr<User> &requestor,
    const dbo::ptr<User> &requestee, const dbo::ptr<Assignment> &assignment) {
  return session.find<Partner_request>()
      .where("requestor_id = ?")
      .bind(requestor.id())
      .where("requestee_id = ?")
      .bind(requestee.id())
      .where("assignment_number = ?")
      .bind(assignment.id());
}

dbo::collection<dbo::ptr<Partner_request>>
Partner_request::find_by_requestor(dbo::Session &session,
                                   const dbo::ptr<User> &requestor) {
  return session.find<Partner_request>()
      .where("requestor_id = ?")
      .bind(requestor.id());
}

dbo::collection<dbo::ptr<Partner_request>>
Partner_request::find_by_requestee(dbo::Session &session,
                                   const dbo::ptr<User> &requestee) {
  return session.find<Partner_request>()
      .where("requestee_id = ?")
      .bind(requestee.id());
}

dbo::ptr<Partner_request> Partner_request::find_by_requestor_and_assignment(
    dbo::Session &session, const dbo::ptr<User> &requestor,
    const dbo::ptr<Assignment> &assignment) {
  return session.find<Partner_request>()
      .where("requestor_id = ?")
      .bind(requestor.id())
      .where("assignment_number = ?")
      .bind(assignment.id());
}

dbo::collection<dbo::ptr<Partner_request>>
Partner_request::find_by_requestee_and_assignment(
    dbo::Session &session, const dbo::ptr<User> &requestee,
    const dbo::ptr<Assignment> &assignment) {
  return session.find<Partner_request>()
      .where("requestee_id = ?")
      .bind(requestee.id())
      .where("assignment_number = ?")
      .bind(assignment.id());
}

void Partner_request::delete_requests(dbo::Session &session,
                                      const dbo::ptr<User> &user,
                                      const dbo::ptr<Assignment> &assignment) {
  auto outgoing = Partner_request::find_by_requestor_and_assignment(
      session, user, assignment);
  if (outgoing)
    outgoing.remove();

  auto incoming = Partner_request::find_by_requestee_and_assignment(
      session, user, assignment);
  for (auto each : incoming) {
    each.remove();
  }
}

bool Partner_request::is_active(dbo::Session &session) const {
  auto submission1 =
      Submission::find_by_assignment_and_user(session, assignment_, requestor_);
  if (submission1 && submission1->user1() == requestor_ &&
      submission1->can_submit(requestor_))
    return true;

  auto submission2 =
      Submission::find_by_assignment_and_user(session, assignment_, requestee_);
  if (submission2 && submission1->user1() == requestee_ &&
      submission2->can_submit(requestee_))
    return true;

  return false;
}

Partner_request::Partner_request(const dbo::ptr<User> &requestor,
                                 const dbo::ptr<User> &requestee,
                                 const dbo::ptr<Assignment> &assignment)
    : requestor_{requestor}, requestee_{requestee}, assignment_{assignment} {}

dbo::ptr<Submission>
Partner_request::confirm(dbo::Session &session, dbo::ptr<User> user1,
                         dbo::ptr<User> user2,
                         dbo::ptr<Assignment> const &assignment) {
  // Alphabetical order
  if (user1->name() > user2->name())
    std::swap(user1, user2);

  auto submission1 =
      Submission::find_by_assignment_and_user(session, assignment, user1);
  auto submission2 =
      Submission::find_by_assignment_and_user(session, assignment, user2);

  bool success = Submission::join_together(submission1, submission2);

  if (success) {
    delete_requests(session, user1, assignment);
    delete_requests(session, user2, assignment);
    return submission1;
  } else {
    throw Withdrawn_partner_request_error{};
  }
}

bool operator<(Partner_request const &a, Partner_request const &b) {
  auto a_num = a.assignment()->number();
  auto b_num = b.assignment()->number();

  if (a_num < b_num)
    return true;
  if (a_num > b_num)
    return false;

  auto const &a_name = a.requestor()->name();
  auto const &b_name = b.requestor()->name();

  if (a_name < b_name)
    return true;
  if (a_name > b_name)
    return false;

  return a.requestee()->name() < b.requestee()->name();
}

char const *
Enum<Partner_request::Status>::show(Partner_request::Status status) {
  using S = Partner_request::Status;
  switch (status) {
  case S::outgoing:
    return "outgoing";
  case S::incoming:
    return "incoming";
  case S::accepted:
    return "accepted";
  case S::canceled:
    return "canceled";
  }
}

namespace rc = std::regex_constants;

static std::regex const outgoing_re("outgoing", rc::icase);
static std::regex const incoming_re("incoming", rc::icase);
static std::regex const accepted_re("accepted", rc::icase);
static std::regex const canceled_re("canceled", rc::icase);

Partner_request::Status
Enum<Partner_request::Status>::read(const char *status) {
  using S = Partner_request::Status;

  auto match = [=](auto re) { return std::regex_match(status, re); };

  if (match(outgoing_re))
    return S::outgoing;
  if (match(incoming_re))
    return S::incoming;
  if (match(accepted_re))
    return S::accepted;
  if (match(canceled_re))
    return S::canceled;

  throw std::invalid_argument{"Could not parse partner request status"};
}

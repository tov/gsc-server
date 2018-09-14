#include "Partner_request.h"
#include "auth/User.h"
#include "Assignment.h"
#include "../Session.h"
#include "Submission.h"

#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(Partner_request)

Wt::Dbo::ptr<Partner_request>
Partner_request::create(Db_session& session,
                        const dbo::ptr<User>& requestor,
                        const dbo::ptr<User>& requestee,
                        const dbo::ptr<Assignment>& assignment,
                        std::string* failure_reason)
{
    auto other_submission = Submission::find_by_assignment_and_user(
            session, assignment, requestee);
    if (other_submission && other_submission->user2()) {
        if (failure_reason) {
            std::ostringstream msg;
            msg << "User " << requestee->name() << " is not available.";
            *failure_reason = msg.str();
        }

        return {};
    }

    if (Partner_request::find_by_requestor_and_assignment(session, requestor,
                                                          assignment)) {
        if (failure_reason) {
            std::ostringstream msg;
            msg << "You already have an outgoing partner request for "
                    << assignment->name() << ".";
            *failure_reason = msg.str();
        }

        return {};
    }

    return session.addNew<Partner_request>(requestor, requestee, assignment);
}

dbo::ptr<Submission>
Partner_request::confirm(Db_session &session) const
{
    // Confirm that the request hasn't been cancelled
    auto self = Partner_request::find_by_requestor_and_requestee(
            session, requestor_, requestee_, assignment_);
    if (!self) return {};

    auto submission1 = Submission::find_by_assignment_and_user(
            session, assignment_, requestor_);
    auto submission2 = Submission::find_by_assignment_and_user(
            session, assignment_, requestee_);

    if (submission1->user2() || submission2->user2())
        return {};

    bool success = Submission::join_together(submission1, submission2);

    if (success) {
        delete_requests(session, requestor_, assignment_);
        delete_requests(session, requestee_, assignment_);
        return submission1;
    } else {
        return {};
    }
}

Wt::Dbo::ptr<Partner_request>
Partner_request::find_by_requestor_and_requestee(
        Db_session &session,
        const Wt::Dbo::ptr<User> &requestor,
        const Wt::Dbo::ptr<User> &requestee,
        const Wt::Dbo::ptr<Assignment> &assignment)
{
    return session.find<Partner_request>()
                  .where("requestor_id = ?").bind(requestor.id())
                  .where("requestee_id = ?").bind(requestee.id())
                  .where("assignment_number = ?").bind(assignment.id());
}

Wt::Dbo::collection<Wt::Dbo::ptr<Partner_request>>
Partner_request::find_by_requestor(Db_session &session,
                                   const dbo::ptr<User> &requestor)
{
    return session.find<Partner_request>()
                  .where("requestor_id = ?").bind(requestor.id());
}

Wt::Dbo::collection<Wt::Dbo::ptr<Partner_request>>
Partner_request::find_by_requestee(Db_session &session,
                                   const dbo::ptr<User> &requestee)
{
    return session.find<Partner_request>()
                  .where("requestee_id = ?").bind(requestee.id());
}


Wt::Dbo::ptr<Partner_request>
Partner_request::find_by_requestor_and_assignment(Db_session& session,
                                                  const Wt::Dbo::ptr<User>& requestor,
                                                  const Wt::Dbo::ptr<Assignment>& assignment)
{
    return session.find<Partner_request>()
                  .where("requestor_id = ?").bind(requestor.id())
                  .where("assignment_number = ?").bind(assignment.id());
}

Wt::Dbo::collection<Wt::Dbo::ptr<Partner_request>>
Partner_request::find_by_requestee_and_assignment(Db_session &session,
                                                  const Wt::Dbo::ptr<User> &requestee,
                                                  const Wt::Dbo::ptr<Assignment> &assignment)
{
    return session.find<Partner_request>()
                  .where("requestee_id = ?").bind(requestee.id())
                  .where("assignment_number = ?").bind(assignment.id());
}

void Partner_request::delete_requests(Db_session &session,
                                      const dbo::ptr<User> &user,
                                      const dbo::ptr<Assignment> &assignment)
{
    auto outgoing = Partner_request::find_by_requestor_and_assignment(session, user, assignment);
    if (outgoing) outgoing.remove();

    auto incoming = Partner_request::find_by_requestee_and_assignment(session, user, assignment);
    for (auto each : incoming) {
        each.remove();
    }
}

bool Partner_request::is_active(Session& session) const
{
    auto submission1 = Submission::find_by_assignment_and_user(
            session, assignment_, requestor_);
    if (submission1 && submission1->user1() == requestor_ &&
            submission1->can_submit(requestor_))
        return true;

    auto submission2 = Submission::find_by_assignment_and_user(
            session, assignment_, requestee_);
    if (submission2 && submission1->user1() == requestee_ &&
        submission2->can_submit(requestee_))
        return true;

    return false;
}

Partner_request::Partner_request(const dbo::ptr<User>& requestor,
                                 const dbo::ptr<User>& requestee,
                                 const dbo::ptr<Assignment>& assignment)
        : requestor_{requestor}
        , requestee_{requestee}
        , assignment_{assignment}
{ }

char const* Enum<Partner_request::Status>::show(Partner_request::Status status)
{
    using S = Partner_request::Status;
    switch (status) {
        case S::outgoing: return "outgoing";
        case S::incoming: return "incoming";
        case S::accepted: return "accepted";
        case S::canceled: return "canceled";
    }
}

namespace rc = std::regex_constants;

static std::regex const outgoing_re("outgoing", rc::icase);
static std::regex const incoming_re("incoming", rc::icase);
static std::regex const accepted_re("accepted", rc::icase);
static std::regex const canceled_re("canceled", rc::icase);

Partner_request::Status Enum<Partner_request::Status>::read(char const* status)
{
    using S = Partner_request::Status;

    if (std::regex_match(status, outgoing_re))
        return S::outgoing;

    if (std::regex_match(status, incoming_re))
        return S::incoming;

    if (std::regex_match(status, accepted_re))
        return S::accepted;

    if (std::regex_match(status, canceled_re))
        return S::canceled;

    throw std::invalid_argument{"Could not parse partner request status"};
}

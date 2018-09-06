#include "Partner_request.h"
#include "auth/User.h"
#include "Assignment.h"
#include "Session.h"
#include "Submission.h"

#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(Partner_request);

dbo::ptr<Partner_request>
Partner_request::create(Session& session,
                        const dbo::ptr<User>& requestor,
                        const dbo::ptr<User>& requestee,
                        const dbo::ptr<Assignment>& assignment)
{
    auto other_submission = Submission::find_by_assignment_and_user(
            session, assignment, requestee);
    if (other_submission && other_submission->user2())
        return {};

    if (Partner_request::find_by_requestor_and_assignment(session, requestor,
                                                          assignment))
        return {};

    auto request = new Partner_request;
    request->requestor_ = requestor;
    request->requestee_ = requestee;
    request->assignment_ = assignment;
    return session.add(request);
}

dbo::ptr<Submission>
Partner_request::confirm(Session& session) const
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
        Session& session,
        const Wt::Dbo::ptr<User>& requestor,
        const Wt::Dbo::ptr<User>& requestee,
        const Wt::Dbo::ptr<Assignment>& assignment)
{
    return session.find<Partner_request>()
                  .where("requestor_id = ?").bind(requestor.id())
                  .where("requestee_id = ?").bind(requestee.id())
                  .where("assignment_number = ?").bind(assignment.id());
}

Wt::Dbo::collection<Wt::Dbo::ptr<Partner_request>>
Partner_request::find_by_requestor(Session& session,
                                   const dbo::ptr<User>& requestor)
{
    return session.find<Partner_request>()
                  .where("requestor_id = ?").bind(requestor.id());
}

Wt::Dbo::collection<Wt::Dbo::ptr<Partner_request>>
Partner_request::find_by_requestee(Session& session,
                                   const dbo::ptr<User>& requestee)
{
    return session.find<Partner_request>()
                  .where("requestee_id = ?").bind(requestee.id());
}


Wt::Dbo::ptr<Partner_request>
Partner_request::find_by_requestor_and_assignment(Session& session,
                                                  const Wt::Dbo::ptr<User>& requestor,
                                                  const Wt::Dbo::ptr<Assignment>& assignment)
{
    return session.find<Partner_request>()
                  .where("requestor_id = ?").bind(requestor.id())
                  .where("assignment_number = ?").bind(assignment.id());
}

Wt::Dbo::collection<Wt::Dbo::ptr<Partner_request>>
Partner_request::find_by_requestee_and_assignment(Session& session,
                                                  const Wt::Dbo::ptr<User>& requestee,
                                                  const Wt::Dbo::ptr<Assignment>& assignment)
{
    return session.find<Partner_request>()
                  .where("requestee_id = ?").bind(requestee.id())
                  .where("assignment_number = ?").bind(assignment.id());
}

void Partner_request::delete_requests(Session& session,
                                      const dbo::ptr <User>& user,
                                      const dbo::ptr <Assignment>& assignment)
{
    session.execute("DELETE FROM partner_requests"
                            " WHERE (requestor_id = ? OR requestee_id = ?)"
                            "   AND assignment_number = ?")
           .bind(user.id()).bind(user.id())
           .bind(assignment.id())
           .run();
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

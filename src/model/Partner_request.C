#include "Partner_request.h"
#include "auth/User.h"
#include "Assignment.h"
#include "Session.h"
#include "Submission.h"

#include <Wt/Dbo/Impl>

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

    if (Partner_request::find_by_requestor(session, requestor, assignment))
        return {};

    auto request = new Partner_request;
    request->requestor_ = requestor;
    request->requestee_ = requestee;
    request->assignment_ = assignment;
    return session.add(request);
}

void Partner_request::confirm(Session& session) const
{
    auto submission1 = Submission::find_by_assignment_and_user(
            session, assignment_, requestor_);
    auto submission2 = Submission::find_by_assignment_and_user(
            session, assignment_, requestee_);

    if (submission1->user2() || submission2->user2())
        return;

    submission1.modify()->set_user2(requestee_);
    submission2.remove();

    session.execute("DELETE FROM partner_requests"
                    " WHERE (requestor_id = ? OR requestor_id = ?"
                    "    OR  requestee_id = ? OR requestee_id = ?)"
                    "   AND assignment_number = ?")
           .bind(requestor_.id()).bind(requestee_.id())
           .bind(requestor_.id()).bind(requestee_.id())
           .bind(assignment_.id())
           .run();
}

Wt::Dbo::ptr<Partner_request>
Partner_request::find_by_requestor(Session& session,
                                   const Wt::Dbo::ptr<User>& requestor,
                                   const Wt::Dbo::ptr<Assignment>& assignment)
{
    return session.find<Partner_request>()
                  .where("assignment_number = ?").bind(assignment.id())
                  .where("requestor_id = ?").bind(requestor.id());
}

Wt::Dbo::collection<Wt::Dbo::ptr<Partner_request>>
Partner_request::find_by_requestee(Session& session,
                                   const Wt::Dbo::ptr<User>& requestee,
                                   const Wt::Dbo::ptr<Assignment>& assignment)
{
    return session.find<Partner_request>()
                  .where("assignment_number = ?").bind(assignment.id())
                  .where("requestee_id = ?").bind(requestee.id());
}


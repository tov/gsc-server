#pragma once

#include "specializations.h"
#include "../common/stringify.h"
#include "../Session.h"

#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/Dbo/Types.h>

namespace dbo = Wt::Dbo;

class Assignment;
class Session;
class Submission;
class User;

class Partner_request
{
public:
    enum class Status {
        outgoing,
        incoming,
        accepted,
        canceled,
    };

    Partner_request() = default;
    Partner_request(const dbo::ptr<User>& requestor,
                    const dbo::ptr<User>& requestee,
                    const dbo::ptr<Assignment>& assignment);

    static Wt::Dbo::ptr<Partner_request>
    create(Db_session&, const dbo::ptr<User>& requestor,
           const dbo::ptr<User>& requestee,
           const dbo::ptr<Assignment>&,
           std::string* failure_reason);

    // Returns the pointer to the joint submission on success.
    dbo::ptr<Submission> confirm(Db_session &) const;

    const dbo::ptr<User>& requestor() const { return requestor_; }
    const dbo::ptr<User>& requestee() const { return requestee_; }
    const dbo::ptr<Assignment>& assignment() const { return assignment_; }

    // Can this Partner_request still be acted on?
    bool is_active(Session&) const;

    static dbo::ptr<Partner_request>
    find_by_requestor_and_requestee(
            Db_session &,
            const dbo::ptr<User> &requestor,
            const dbo::ptr<User> &requestee,
            const dbo::ptr<Assignment> &);

    static dbo::collection<dbo::ptr<Partner_request>>
    find_by_requestor(Db_session &, const dbo::ptr<User> &);

    static dbo::collection<dbo::ptr<Partner_request>>
    find_by_requestee(Db_session &, const dbo::ptr<User> &);

    static dbo::ptr<Partner_request>
    find_by_requestor_and_assignment(Db_session&,
                                     const dbo::ptr<User>&,
                                     const dbo::ptr<Assignment>&);

    static dbo::collection<dbo::ptr<Partner_request>>
    find_by_requestee_and_assignment(Db_session &,
                                     const dbo::ptr<User> &,
                                     const dbo::ptr<Assignment> &);

    static void
    delete_requests(Db_session &,
                    const dbo::ptr<User> &,
                    const dbo::ptr<Assignment> &);

private:
    dbo::ptr<User> requestor_;
    dbo::ptr<User> requestee_;
    dbo::ptr<Assignment> assignment_;

public:
    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, requestor_, "requestor", dbo::OnDeleteCascade);
        dbo::belongsTo(a, requestee_, "requestee", dbo::OnDeleteCascade);
        dbo::belongsTo(a, assignment_, "assignment", dbo::OnDeleteCascade);
    }
};

template <>
struct Enum<Partner_request::Status>
{
    static char const* show(Partner_request::Status);
    static Partner_request::Status read(char const*);
};

DBO_EXTERN_TEMPLATES(Partner_request)

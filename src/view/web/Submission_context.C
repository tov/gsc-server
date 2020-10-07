#include "../../Session.h"
#include "../../common/util.h"
#include "../../model/Submission.h"
#include "Submission_context.h"

namespace detail {

Submission_context_data::Submission_context_data(
    Submission_change_signal &change, Session &session,
    Wt::Dbo::ptr<Submission> const &submission,
    Wt::Dbo::ptr<User> const &principal)
    : changed_(change), session_(session), submission_(submission),
      principal_(principal ? principal : session.user()) {}

Wt::Dbo::ptr<Assignment>
assignment_of_submission(Wt::Dbo::ptr<Submission> const &submission) {
  return submission->assignment();
}

} // end namespace detail

Submission_context::Submission_context(
    Submission_change_signal &changed, Session &session,
    const Wt::Dbo::ptr<Submission> &submission,
    const Wt::Dbo::ptr<User> &principal)
    : detail::Submission_context_data{changed, session, submission, principal} {
  changed_.connect(this, &Submission_context::on_change_);
}

Submission_context::Submission_context(Submission_context const &o)
    : Submission_context{o.changed_, o.session_, o.submission_, o.principal_} {}

Wt::Dbo::ptr<Assignment> Submission_context::assignment() const {
  return detail::assignment_of_submission(submission());
}

void Submission_context::notify(Submission_change_message submission) {
  changed_.emit(std::move(submission));
}

void Submission_context::notify() { changed_.emit(nullptr); }

void Submission_context::on_change_(Submission_change_message submission) {
  if (submission_ && submission)
    submission_ = submission;

  on_change();
}

Submission_context_root::Submission_context_root(
    Session &session, Wt::Dbo::ptr<Submission> const &submission,
    Wt::Dbo::ptr<User> const &principal)
    : detail::Submission_context_root_data{}, Submission_context{
                                                  real_changed_, session,
                                                  submission, principal} {}

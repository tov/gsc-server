#pragma once

#include "../../model/specializations.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WObject.h>
#include <Wt/WSignal.h>

class Assignment;
class User;
class Session;
class Submission;

using Submission_change_message = Wt::Dbo::ptr<Submission>;
using Submission_change_signal = Wt::Signal<Submission_change_message>;

namespace detail {

struct Submission_context_data {
  Submission_change_signal &changed_;
  Session &session_;
  Wt::Dbo::ptr<Submission> submission_;
  Wt::Dbo::ptr<User> principal_;

  Submission_context_data(Submission_change_signal &, Session &,
                          Wt::Dbo::ptr<Submission> const &,
                          Wt::Dbo::ptr<User> const &);
};

struct Submission_context_root_data {
  Submission_change_signal real_changed_;
};

Wt::Dbo::ptr<Assignment>
assignment_of_submission(Wt::Dbo::ptr<Submission> const &);

} // end namespace detail

class Submission_context : private detail::Submission_context_data,
                           public Wt::Core::observable {
public:
  Wt::Dbo::ptr<Submission> const &submission() const { return submission_; }
  Wt::Dbo::ptr<User> const &principal() const { return principal_; }
  Wt::Dbo::ptr<Assignment> assignment() const;

  Session &session() { return session_; }
  void notify(Submission_change_message submission);
  void notify();

  Submission_context &context() { return *this; }

protected:
  Submission_context(Submission_change_signal &, Session &,
                     Wt::Dbo::ptr<Submission> const &,
                     Wt::Dbo::ptr<User> const & = nullptr);

  Submission_context(Submission_context const &);

  Submission_change_signal &changed() { return changed_; }

  virtual void on_change() = 0;

private:
  void on_change_(Submission_change_message);
};

class Submission_context_root : private detail::Submission_context_root_data,
                                public Submission_context {
protected:
  Submission_context_root(Session &, Wt::Dbo::ptr<Submission> const &,
                          Wt::Dbo::ptr<User> const & = nullptr);
};

#pragma once

#include "../Submission_context.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>

class Session;
class Submission;

class Submission_owner_widget : public Wt::WCompositeWidget,
                                private Submission_context {
public:
  Submission_owner_widget(Submission_context const &);

  Submission_owner_widget(Wt::Dbo::ptr<Submission> const &, Session &,
                          Submission_change_signal &);

private:
  Wt::WContainerWidget *impl_;

  void load_();

  void reload_();
  void update_admin_();
  void update_grader_();
  void update_student_();

  void break_up_partnership_();

  void on_change() final override;
};
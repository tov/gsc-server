#pragma once

#include "../Submission_context.h"

#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>

class Partner_notification_widget : public Wt::WCompositeWidget,
                                    private Submission_context {
public:
  Partner_notification_widget(Wt::Dbo::ptr<User> const &,
                              Wt::Dbo::ptr<Submission> const &, Session &,
                              Submission_change_signal &);

protected:
  void on_change() final override { reload_(); }

private:
  Wt::WContainerWidget *impl_;

  void reload_();
};

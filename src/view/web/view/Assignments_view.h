#pragma once

#include "../../../Session.h"
#include "../../../model/Assignment.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WTable.h>

#include <memory>
#include <vector>

class Date_time_edit;

class Assignments_view_row {
public:
  Assignments_view_row(const dbo::ptr<Assignment> &, Session &,
                       Wt::WTableRow *);

  enum columns {
    NUMBER,
    NAME,
    PARTNER,
    WEB_ALLOWED,
    OPEN_DATE,
    DUE_DATE,
    EVAL_DATE,
    POINTS,
    ACTION,
  };

  static void add_headings(Wt::WTableRow *);

private:
  dbo::ptr<Assignment> assignment_;
  Session &session_;
  Wt::WTableRow *row_;
  Wt::WCheckBox *partner_;
  Wt::WCheckBox *web_allowed_;
  Wt::WLineEdit *name_;
  Date_time_edit *open_date_;
  Date_time_edit *due_date_;
  Date_time_edit *eval_date_;
  Wt::WLineEdit *points_;

  void update_() const;

  friend class Assignments_view;
};

class Assignments_view : public Wt::WContainerWidget {
public:
  Assignments_view(Session &session);

private:
  Session &session_;
  std::vector<std::unique_ptr<Assignments_view_row>> rows_;
  Wt::WTable *table_;

  void more_();

  void fewer_();
  void real_fewer_();
};

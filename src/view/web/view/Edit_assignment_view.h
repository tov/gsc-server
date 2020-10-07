#pragma once

#include "../../../model/Assignment.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WContainerWidget.h>

#include <vector>

class Session;

class Edit_eval_item;

class Edit_assignment_view : public Wt::WContainerWidget {
public:
  Edit_assignment_view(const dbo::ptr<Assignment> &, Session &);

  void reload_all();

private:
  dbo::ptr<Assignment> assignment_;
  Session &session_;

  Wt::WContainerWidget *container_;
  std::vector<Edit_eval_item *> items_;

  void more_();
  void add_item_(const dbo::ptr<Eval_item> &);

  void fewer_();
  void real_fewer_();
};

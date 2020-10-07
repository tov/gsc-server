#pragma once

#include "Abstract_file_view.h"
#include <Wt/Dbo/ptr.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>

class Submission;
class User;

class Evaluation_view : public File_view_base {
public:
  Evaluation_view(const Wt::Dbo::ptr<Submission> &, Session &);

  void go_to(unsigned int);
  void go_default();

  // For evaluation_list_view_item:
  using File_view_base::submission;

  // Is the current user allowed to change the self evaluation right now?
  bool can_eval();

private:
  void load_();
};

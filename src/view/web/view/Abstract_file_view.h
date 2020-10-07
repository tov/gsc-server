#pragma once

#include "../Reloadable.h"
#include "../Submission_context.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>

class File_viewer;

class Submission;
class Session;

class Abstract_file_view : public Wt::WCompositeWidget,
                           protected Submission_context_root {
public:
  Abstract_file_view(const Wt::Dbo::ptr<Submission> &, Session &);

  File_viewer *file_viewer() { return viewer_; }

protected:
  File_viewer *viewer_;
  Wt::WContainerWidget *right_column_;
};

class File_view_base : public Abstract_file_view {
public:
  using Abstract_file_view::Abstract_file_view;

protected:
  void on_change() override {}
};

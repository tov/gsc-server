#pragma once

#include "../Reloadable.h"
#include "../Submission_context.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WResource.h>

class File_meta;
class Session;
class Submission;

namespace Wt {
class WTable;
class WTableCell;
} // namespace Wt

// A list of source files that allows downloading and deleting.
class File_list_widget : public Wt::WCompositeWidget,
                         private Submission_context {
public:
  File_list_widget(const Wt::Dbo::ptr<Submission> &, bool can_modify,
                   Session &session, Submission_change_signal &);

private:
  bool can_modify_;
  Wt::WTable *inner_;

  void reload_();

  void on_change() final override { reload_(); }

  Wt::WTableCell *cell_(int, int);
};

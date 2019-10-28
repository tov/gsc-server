#pragma once

#include "Reloadable.h"
#include "Submission_context.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WResource.h>
#include <Wt/WTable.h>

class File_meta;
class Session;
class Submission;

// A list of source files that allows downloading and deleting.
class File_list_widget : public Wt::WTable,
                         private Submission_context
{
public:
    File_list_widget(const Wt::Dbo::ptr<Submission>&,
                     bool can_modify,
                     Session& session,
                     Submission_change_signal&);

private:
    bool can_modify_;

    void reload_();

    void on_change() final override { reload_(); }

};

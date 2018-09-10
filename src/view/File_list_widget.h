#pragma once

#include "Reloadable.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WResource.h>
#include <Wt/WTable.h>

class File_meta;
class Session;
class Submission;

// A list of source files that allows downloading and deleting.
class File_list_widget : public Wt::WTable, public Reloadable
{
public:
    File_list_widget(const Wt::Dbo::ptr<Submission>&, Session& session,
                     Wt::Signal<>* changed = nullptr);

    void reload() override;

private:
    Wt::Dbo::ptr<Submission> submission_;
    Session& session_;
    Wt::Signal<>* changed_;
};

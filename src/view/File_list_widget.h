#pragma once

#include "Reloadable.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WResource.h>
#include <Wt/WTable.h>

class File_meta;
class Session;
class Submission;

// Action for deleting a source file.
class File_deleter : public Wt::WObject
{
public:
    File_deleter(const Wt::Dbo::ptr<File_meta>& source_file,
                 Wt::Signal<>* changed,
                 Session& session);

    void go();

private:
    Wt::Signal<>* changed_;
    Wt::Dbo::ptr<File_meta> source_file_;
    Session& session_;
};

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
    std::vector<std::unique_ptr<File_deleter>> deleters_;
};

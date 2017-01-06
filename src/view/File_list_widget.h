#pragma once

#include <Wt/Dbo/ptr>
#include <Wt/WResource>
#include <Wt/WTable>

class File_deleter;

class File_meta;
class Session;
class Submission;

// A list of source files that allows downloading and deleting.
class File_list_widget : public Wt::WTable
{
public:
    File_list_widget(const Wt::Dbo::ptr<Submission>&,
                     Session& session,
                     Wt::WContainerWidget* parent = nullptr);

    void reload();

    // This signal is a two-way channel--it fires when the File_list_widget
    // changes, and firing it will cause the File_list_widget to reload.
    // Is this weird?
    Wt::Signal<>& changed() { return changed_; };

private:
    Wt::Dbo::ptr<Submission> submission_;
    Session& session_;

    Wt::Signal<> changed_;
    std::vector<std::unique_ptr<File_deleter>> deleters_;
};

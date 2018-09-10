#pragma once

#include "Abstract_file_view.h"

class Reloadable {
public:
    virtual void reload() = 0;
};

class File_manager_view : public Abstract_file_view, public Reloadable
{
public:
    File_manager_view(const Wt::Dbo::ptr<Submission>&, Session&);

    void reload() override;

private:
    Reloadable* quota_display_;
    Reloadable* date_list_;
};

#pragma once

#include "Reloadable.h"
#include "Abstract_file_view.h"

class Date_list;
class File_list_widget;
class Quota_display;

class File_manager_view : public Abstract_file_view
{
public:
    File_manager_view(const Wt::Dbo::ptr<Submission>&, Session&);

    void reload() override;

private:
    Date_list* date_list_;
    File_list_widget* file_list_;
    Quota_display* quota_display_;
};

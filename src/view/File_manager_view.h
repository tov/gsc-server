#pragma once

#include "Abstract_file_view.h"

#include <Wt/WContainerWidget.h>

class File_manager_view : public Abstract_file_view
{
public:
    File_manager_view(const Wt::Dbo::ptr<Submission>&,
                      Session&,
                      Wt::WContainerWidget* parent = nullptr);
};

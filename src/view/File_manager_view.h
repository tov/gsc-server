#pragma once

#include "Abstract_file_view.h"

class File_manager_view : public Abstract_file_view
{
public:
    File_manager_view(const Wt::Dbo::ptr<Submission>&, Session&);
};

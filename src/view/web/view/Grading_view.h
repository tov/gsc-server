#pragma once

#include "Abstract_file_view.h"

#include <Wt/Dbo/ptr.h>

class Self_eval;
class Session;

class Grading_view : public File_view_base
{
public:
    Grading_view(Wt::Dbo::ptr<Self_eval> const&, Session&);

private:
    Wt::Dbo::ptr<Self_eval> model_;
};

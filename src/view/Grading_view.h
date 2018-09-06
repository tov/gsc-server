#pragma once

#include "Abstract_file_view.h"

#include <Wt/Dbo/ptr.h>
#include <Wt/WContainerWidget.h>

class Self_eval;
class Session;

class Grading_view : public Abstract_file_view
{
public:
    Grading_view(const Wt::Dbo::ptr<Self_eval>,
                 Session&,
                 Wt::WContainerWidget* parent = nullptr);

private:
    Wt::Dbo::ptr<Self_eval> model_;
};

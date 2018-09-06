#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/Dbo/ptr.h>

class Session;

class Admin_view : public Wt::WContainerWidget
{
public:
    Admin_view(Session&,
               WContainerWidget* parent = nullptr);

private:
    Session& session_;
};

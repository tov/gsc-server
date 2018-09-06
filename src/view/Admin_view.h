#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/Dbo/ptr.h>

class Session;

class Admin_view : public Wt::WContainerWidget
{
public:
    explicit Admin_view(Session&);

private:
    Session& session_;
};

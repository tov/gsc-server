#pragma once

#include <Wt/WContainerWidget>
#include <Wt/Dbo/ptr>

class Session;

class Admin_view : public Wt::WContainerWidget
{
public:
    Admin_view(Session&,
               WContainerWidget* parent = nullptr);

private:
    Session& session_;
};

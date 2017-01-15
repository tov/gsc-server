#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>

class Session;

class Held_back_view : public Wt::WCompositeWidget
{
public:
    Held_back_view(Session&,
                   Wt::WContainerWidget* parent = nullptr);

private:
    Session& session_;
};
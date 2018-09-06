#pragma once

#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>

class Session;

class Held_back_view : public Wt::WCompositeWidget
{
public:
    Held_back_view(Session&,
                   Wt::WContainerWidget* parent = nullptr);

private:
    Session& session_;
};
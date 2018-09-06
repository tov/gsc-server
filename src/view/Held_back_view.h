#pragma once

#include <Wt/WCompositeWidget.h>

class Session;

class Held_back_view : public Wt::WCompositeWidget
{
public:
    explicit Held_back_view(Session&);

private:
    Session& session_;
};
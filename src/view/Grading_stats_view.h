#pragma once

#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>

class Session;

class Grading_stats_view : public Wt::WCompositeWidget
{
public:
    Grading_stats_view(Session&, Wt::WContainerWidget* = nullptr);

private:
    Session& session_;
};
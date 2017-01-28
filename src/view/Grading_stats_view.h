#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>

class Session;

class Grading_stats_view : public Wt::WCompositeWidget
{
public:
    Grading_stats_view(Session&, Wt::WContainerWidget* = nullptr);

private:
    Session& session_;
};
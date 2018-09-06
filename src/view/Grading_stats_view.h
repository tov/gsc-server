#pragma once

#include <Wt/WCompositeWidget.h>

class Session;

class Grading_stats_view : public Wt::WCompositeWidget
{
public:
    explicit Grading_stats_view(Session&);

private:
    Session& session_;
};
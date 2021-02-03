#pragma once

#include <Wt/WCompositeWidget.h>

class Session;

class Notable_self_evals_view : public Wt::WCompositeWidget
{
public:
    explicit Notable_self_evals_view(Session&);

private:
    Session& session_;
};

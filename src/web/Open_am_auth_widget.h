#pragma once

#include <Wt/WCompositeWidget.h>
#include <Wt/WTemplate.h>

class Session;

class Open_am_auth_widget : public Wt::WCompositeWidget
{
public:
    explicit Open_am_auth_widget(Session&);

    void reload();

private:
    Wt::WTemplate* impl_;
    Session& session_;
};


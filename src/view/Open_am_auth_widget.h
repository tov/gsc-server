#pragma once

#include <Wt/WContainerWidget.h>

class Session;

class Open_am_auth_widget : public Wt::WContainerWidget
{
public:
    explicit Open_am_auth_widget(Session&);

    void reload();

private:
    Session& session_;
};


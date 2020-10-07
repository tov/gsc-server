#pragma once

#include <Wt/Dbo/ptr.h>
#include <Wt/WContainerWidget.h>

class Session;

class Admin_view : public Wt::WContainerWidget {
public:
  explicit Admin_view(Session &);

private:
  Session &session_;
};

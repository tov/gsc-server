#pragma once

#include <Wt/WCompositeWidget.h>
#include <Wt/WTemplate.h>

class Session;

class Auth_widget : public Wt::WCompositeWidget {
public:
  explicit Auth_widget(Session &);

  void reload();

private:
  Wt::WTemplate *impl_;
  Session &session_;
};

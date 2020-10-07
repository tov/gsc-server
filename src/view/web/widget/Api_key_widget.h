#pragma once

#include <Wt/Dbo/ptr.h>
#include <Wt/WCompositeWidget.h>

namespace Wt {
class WTemplate;
}

class User;
class Session;

class Api_key_widget : public Wt::WCompositeWidget {
public:
  Api_key_widget(Wt::Dbo::ptr<User> const &, Session &);

private:
  void load_();

  void copy_() const;
  void confirm_regenerate_();
  void regenerate_();

  Wt::Dbo::ptr<User> user_;
  Session &session_;
  Wt::WTemplate *impl_;
};

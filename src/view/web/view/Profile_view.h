#pragma once

#include <Wt/Dbo/ptr.h>
#include <Wt/WCompositeWidget.h>

namespace Wt {
class WTemplate;
}

class User;
class Session;

class Profile_view : public Wt::WCompositeWidget {
public:
  Profile_view(Wt::Dbo::ptr<User> const &, Session &);

  class Call_me_widget;

private:
  Wt::Dbo::ptr<User> user_;
  Session &session_;
  Wt::WTemplate *impl_;
};

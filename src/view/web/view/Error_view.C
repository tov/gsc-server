#include "Error_view.h"

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

Error_view::Error_view(const std::string &message) {
  setContentAlignment(Wt::AlignmentFlag::Center);

  addNew<Wt::WText>(message);
  addNew<Wt::WBreak>();

  auto button = addNew<Wt::WPushButton>("Home");
  button->clicked().connect(std::bind(
      [=]() { Wt::WApplication::instance()->setInternalPath("/", true); }));
}

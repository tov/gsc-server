#include "Notification.h"

#include <Wt/WApplication.h>
#include <Wt/WMessageBox.h>

static void no_op() {}

Notification::Notification(std::string const &title, Wt::Icon icon)
    : title_{title}, icon_{icon}, callback_{&no_op} {}

Notification::~Notification() {
  auto box = Wt::WApplication::instance()->root()->addNew<Wt::WMessageBox>(
      title_, message_.str(), icon_, Wt::StandardButton::Ok);

  box->setModal(true);
  box->buttonClicked().connect([box = box, callback = std::move(callback_)] {
    box->setModal(false);
    box->hide();
    box->removeFromParent();
    callback();
  });
  box->show();
}

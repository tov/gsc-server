#include "Confirmation_dialog.h"

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

Confirmation_dialog &Confirmation_dialog::create(Wt::WString const &message) {
  auto root = Wt::WApplication::instance()->root();
  std::unique_ptr<Confirmation_dialog> ptr(new Confirmation_dialog(message));
  return *root->addChild(std::move(ptr));
}

Confirmation_dialog::Confirmation_dialog(Wt::WString const &message)
    : WDialog("Confirm action") {
  contents()->addNew<Wt::WText>(message);
  contents()->addNew<Wt::WBreak>();

  auto buttons = contents()->addNew<Wt::WContainerWidget>();
  buttons->setStyleClass("dialog-buttons");

  auto no = buttons->addNew<Wt::WPushButton>("No");
  auto yes = buttons->addNew<Wt::WPushButton>("Yes");
  no->clicked().connect(this, &Wt::WDialog::reject);
  yes->clicked().connect(this, &Wt::WDialog::accept);

  setModal(true);
  rejectWhenEscapePressed();
  finished().connect(this, &Confirmation_dialog::finished_);
  show();
}

void Confirmation_dialog::finished_(Wt::DialogCode code) {
  if (code == Wt::DialogCode::Accepted)
    accepted_.emit();
  else
    rejected_.emit();

  removeFromParent();
}

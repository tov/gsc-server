#include "Confirmation_dialog.h"

#include <Wt/WBreak.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

Confirmation_dialog::Confirmation_dialog(const std::string& message,
                                         Wt::WObject* parent)
        : WDialog("Confirm action", parent)
{
    new Wt::WText(message, contents());
    new Wt::WBreak(contents());

    auto buttons = new Wt::WContainerWidget(contents());
    buttons->setStyleClass("dialog-buttons");

    auto no  = new Wt::WPushButton("No", buttons);
    auto yes = new Wt::WPushButton("Yes", buttons);
    no->clicked().connect(this, &Wt::WDialog::reject);
    yes->clicked().connect(this, &Wt::WDialog::accept);

    rejectWhenEscapePressed();
    finished().connect(this, &Confirmation_dialog::finished_);
    show();
}

void Confirmation_dialog::finished_(Wt::WDialog::DialogCode code)
{
    if (code == Wt::WDialog::Accepted)
        accepted_.emit();
    else
        rejected_.emit();

    delete this;
}

#include "Notification.h"

#include <Wt/WMessageBox.h>

static void no_op() { }

Notification::Notification(std::string const& title, Wt::WWidget* parent)
        : title_{title}
        , parent_{parent}
        , callback_{&no_op}
{ }

Notification::~Notification()
{
    auto unique_box = std::make_unique<Wt::WMessageBox>(
            title_,
            message_.str(),
            Wt::Icon::Critical,
            Wt::StandardButton::Ok);
    auto callback = std::move(callback_);
    auto box = parent_->addChild(std::move(unique_box));
    box->setModal(true);
    box->buttonClicked().connect([=] {
        box->hide();
        box->removeFromParent();
        callback();
    });
    box->show();
}

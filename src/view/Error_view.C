#include "Error_view.h"

#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WPushButton>
#include <Wt/WText>

Error_view::Error_view(const std::string& message)
{
    setContentAlignment(Wt::AlignCenter);

    new Wt::WText(message, this);
    new Wt::WBreak(this);

    auto button = new Wt::WPushButton("Home", this);
    button->clicked().connect(std::bind([=]() {
        Wt::WApplication::instance()->setInternalPath("/", true);
    }));
}

#include "Auth_widget.h"
#include "Glyph_button.h"
#include "../../../Config.h"
#include "../../../Session.h"
#include "../../../Navigate.h"
#include "../../../common/env_var.h"

#include <Wt/Dbo/Transaction.h>
#include <Wt/Utils.h>
#include <Wt/WApplication.h>
#include <Wt/WPushButton.h>
#include <Wt/WString.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

#include <string>
#include <sstream>

using namespace Wt;
namespace dbo = Wt::Dbo;

namespace {

std::string login_url()
{
    auto base_url = CONFIG().gsc_base_url().c_str();

    auto buf = std::ostringstream();
    buf << CONFIG().websso_login_url() << "?goto=";
    buf << Utils::urlEncode(base_url);
    buf << Utils::urlEncode(WApplication::instance()->bookmarkUrl());
    return buf.str();
}

}

void Auth_widget::reload()
{
    impl_->clear();

    dbo::Transaction transaction(session_);

    auto add_button = [=](auto key, auto icon, auto title,
                          auto const& destination)
    {
        auto button = impl_->bindNew<Glyph_button>(key, icon, title, false);
        button->setStyleClass("btn btn-lg btn-info");
        button->clicked().connect(destination);
    };

    if (session_.authenticate_from_environment()) {
        impl_->setTemplateText(tr("gsc.template.logged-in"));
        impl_->bindString("user-name", session_.user()->nice_name(false));

        add_button("profile-link", "user", "Profile",
                   Navigate{session_.user()->profile_url()});
        add_button("log-out-link", "log-out", "Log out",
                   Navigate{CONFIG().websso_logout_url(), true});
    } else {
        impl_->setTemplateText(tr("gsc.template.not-logged-in"));

        add_button("log-in-link", "log-in", "Log in",
                   Navigate{login_url(), true});
    }
}

Auth_widget::Auth_widget(Session& session)
        : WCompositeWidget{std::make_unique<WTemplate>()}
        , impl_{dynamic_cast<WTemplate*>(implementation())}
        , session_{session}
{
    reload();
}


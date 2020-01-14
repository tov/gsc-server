#include "Auth_widget.h"
#include "Glyph_button.h"
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

char const* const LOGOUT_URL =
        "https://websso.it.northwestern.edu/amserver/UI/Logout";
char const* const LOGIN_URL_BASE =
        "https://websso.it.northwestern.edu/amserver/UI/Login";

std::string login_url()
{
    auto base_url = get_env_var("GSC_BASE_URL", "http://localhost:9090/");

    auto buf = std::ostringstream();
    buf << LOGIN_URL_BASE << "?goto=";
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
        impl_->bindString("user-name", session_.user_name());

        add_button("profile-link", "user", "Profile",
                   Navigate{session_.user()->profile_url()});
        add_button("log-out-link", "log-out", "Log out",
                   Navigate{LOGOUT_URL, true});
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


#include "Open_am_auth_widget.h"
#include "../../../Session.h"
#include "../../../Navigate.h"

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

std::string const BASE_URL {
        "https://cs214.cs.northwestern.edu/"
};

char const* const LOGOUT_URL =
        "https://websso.it.northwestern.edu/amserver/UI/Logout";
char const* const LOGIN_URL_BASE =
        "https://websso.it.northwestern.edu/amserver/UI/Login";

Navigate const redirect_logout { LOGOUT_URL, true };

Navigate redirect_login()
{
    auto buf = std::ostringstream();
    buf << LOGIN_URL_BASE << "?goto=";
    buf << Utils::urlEncode(BASE_URL);
    buf << Utils::urlEncode(WApplication::instance()->bookmarkUrl());
    return Navigate(buf.str(), true);
}

}

void Open_am_auth_widget::reload()
{
    impl_->clear();

    dbo::Transaction transaction(session_);

    if (session_.authenticate_from_environment()) {
        impl_->setTemplateText(tr("gsc.template.logged-in"));
        impl_->bindString("user-name", session_.user_name());
        impl_->bindNew<WPushButton>("logout", "Logout")
                ->clicked().connect(redirect_logout);
    } else {
        impl_->setTemplateText(tr("gsc.template.not-logged-in"));
        impl_->bindNew<WPushButton>("login", "Login")
                ->clicked().connect(redirect_login());
    }
}

Open_am_auth_widget::Open_am_auth_widget(Session& session)
        : WCompositeWidget{std::make_unique<WTemplate>()}
        , impl_{dynamic_cast<WTemplate*>(implementation())}
        , session_{session}
{
    reload();
}


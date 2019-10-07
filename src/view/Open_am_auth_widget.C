#include "Open_am_auth_widget.h"
#include "../Session.h"

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

WString const LOGGED_IN_TEMPLATE {
        "<p>Logged in as <strong>${name}</strong> | ${logout}</p>"
};

WString const LOGGED_OUT_TEMPLATE {
        "<p>Not logged in | ${login}</p>"
};

std::string const LOGOUT_URL {
        "https://websso.it.northwestern.edu/amserver/UI/Logout"
};

std::string const LOGIN_URL_BASE {
        "https://websso.it.northwestern.edu/amserver/UI/Logout"
};

std::string const BASE_URL {
        "https://cs124.cs.northwestern.edu/"
};

void redirect_login()
{
    auto app = WApplication::instance();
    auto buf = std::ostringstream();

    buf << LOGIN_URL_BASE << "?goto=";
    buf << Utils::urlEncode(BASE_URL + app->bookmarkUrl());

    app->redirect(buf.str());
}

}

void Open_am_auth_widget::reload()
{
    clear();

    dbo::Transaction transaction(session_);

    if (session_.authenticate_from_environment()) {
        auto div = addNew<WTemplate>(LOGGED_IN_TEMPLATE);
        div->bindNew<WText>("name", session_.user_name());
        div->bindNew<WPushButton>("logout", "Logout")->clicked().connect([&]() {
            WApplication::instance()->redirect(LOGOUT_URL);
        });
    } else {
        auto div = addNew<WTemplate>(LOGGED_OUT_TEMPLATE);
        div->bindNew<WPushButton>("login", "Login")->clicked().connect([&]() {
            redirect_login();
        });
    }
}

Open_am_auth_widget::Open_am_auth_widget(Session& session)
        : session_{session}
{
    setStyleClass("Wt-auth-logged-in");
    reload();
}


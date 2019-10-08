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


// TODO DELETE {
#include "../common/env_var.h"
#include <cstdlib>
#include <fstream>
#include <Wt/WEnvironment>
struct say
{
public:
    say() = default;
    //
    template<class T>
    say(T const& v)
    {
        *this << v;
    }
    //
    template<class T>
    say& operator<<(T const& v)
    {
        buf_ << v;
        return *this;
    }
    //
    ~say() noexcept(false)
    {
        std::cerr << "***** ";
        std::cerr << buf_.str();
        std::cerr << "\n";
    }
    //
private:
    std::ostringstream buf_;
};
void header(std::string const& name) {
    auto const& env = Wt::WApplication::instance()->environment();
    auto value = env.headerValue(name);
    say() << "HEADER: " << name << ": " << value;
};
// TODO DELETE }


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
        "https://cs214.cs.northwestern.edu/"
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

    say("MARK 0");
    if (session_.authenticate_from_environment()) {
        say("MARK 1");
        auto div = addNew<WTemplate>(LOGGED_IN_TEMPLATE);
        say("MARK 2") << session_.user_name();
        div->bindNew<WText>("name", session_.user_name());
        say("MARK 3");
        div->bindNew<WPushButton>("logout", "Logout")->clicked().connect([&]() {
            WApplication::instance()->redirect(LOGOUT_URL);
        });

        say("MARK 4");

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


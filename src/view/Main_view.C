#include "Main_view.h"
#include "Open_am_auth_widget.h"
#include "game/HangmanWidget.h"
#include "game/HighScoresWidget.h"
#include "Admin_view.h"

#include <Wt/Auth/AuthWidget.h>
#include <Wt/WApplication.h>
#include <Wt/WAnchor.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

using namespace Wt;

Main_view::Main_view(Session& session)
        : session_(session)
{
    auto h1 = addNew<WTemplate>("<h1>${home} <small>${title}</small></h1>");

    auto home = h1->bindWidget("home", std::make_unique<WText>("gsc"));
    home->clicked().connect(std::bind([=]() {
        WApplication::instance()->setInternalPath("/", true);
    }));
    home->setToolTip("Go home");
    home->setStyleClass("home-link");

    title_ = h1->bindWidget("title",
                            std::make_unique<WText>("Homework Server"));

    root_ = addNew<WContainerWidget>();

#if defined(GSC_AUTH_OPEN_AM)
    addNew<Open_am_auth_widget>(session_);
#elif defined(GSC_AUTH_PASSWORD)
    auto authModel = std::make_unique<Auth::AuthModel>(
            Session::auth(), session_.users());
    authModel->addPasswordAuth(&Session::passwordAuth());

    auto authWidget = addNew<Auth::AuthWidget>(session_.login());
    authWidget->setModel(std::move(authModel));
    authWidget->setRegistrationEnabled(false);
    authWidget->processEnvironment();
#endif
}

void Main_view::set_title(const WString& title)
{
    title_->setText(title);
}

void Main_view::set_widget(std::unique_ptr<WWidget> new_widget)
{
    root_->clear();

    if (new_widget != nullptr)
        root_->addWidget(std::move(new_widget));
}

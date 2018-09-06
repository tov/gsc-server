#include "Main_view.h"
#include "game/HangmanWidget.h"
#include "game/HighScoresWidget.h"
#include "Admin_view.h"

#include <Wt/Auth/AuthWidget.h>
#include <Wt/WApplication.h>
#include <Wt/WAnchor.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>

using namespace Wt;

Main_view::Main_view(Session& session, WContainerWidget* parent)
        : WContainerWidget(parent), session_(session)
{
    auto h1 = new WTemplate("<h1>${home} <small>${title}</small></h1>", this);

    auto home = new Wt::WText("gsc");
    h1->bindWidget("home", home);
    home->clicked().connect(std::bind([=]() {
        Wt::WApplication::instance()->setInternalPath("/", true);
    }));
    home->setToolTip("Go home");
    home->setStyleClass("home-link");

    title_ = new Wt::WText("Homework Server");
    h1->bindWidget("title", title_);

    root_ = new Wt::WContainerWidget(this);

    Auth::AuthModel* authModel = new Auth::AuthModel(Session::auth(),
                                                     session_.users(), this);
    authModel->addPasswordAuth(&Session::passwordAuth());

    Auth::AuthWidget* authWidget = new Auth::AuthWidget(session_.login(), this);
    authWidget->setModel(authModel);
    authWidget->setRegistrationEnabled(true);
    authWidget->processEnvironment();
}

void Main_view::set_title(const Wt::WString& title)
{
    title_->setText(title);
}

void Main_view::set_widget(Wt::WWidget* new_widget)
{
    root_->clear();

    if (new_widget != nullptr)
        root_->addWidget(new_widget);
}


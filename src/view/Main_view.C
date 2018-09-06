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
        : session_(session)
{
    auto h1 = addNew<WTemplate>("<h1>${home} <small>${title}</small></h1>");

    auto home = h1->bindWidget("home", std::make_unique<Wt::WText>("gsc"));
    home->clicked().connect(std::bind([=]() {
        Wt::WApplication::instance()->setInternalPath("/", true);
    }));
    home->setToolTip("Go home");
    home->setStyleClass("home-link");

    title_ = h1->bindWidget("title",
                            std::make_unique<Wt::WText>("Homework Server"));

    root_ = addNew<Wt::WContainerWidget>();

    auto authModel = std::make_unique<Auth::AuthModel>(
            Session::auth(), session_.users());
    authModel->addPasswordAuth(&Session::passwordAuth());

    auto authWidget = addNew<Auth::AuthWidget>(session_.login());
    authWidget->setModel(std::move(authModel));
    authWidget->setRegistrationEnabled(true);
    authWidget->processEnvironment();
}

void Main_view::set_title(const Wt::WString& title)
{
    title_->setText(title);
}

void Main_view::set_widget(std::unique_ptr<Wt::WWidget> new_widget)
{
    root_->clear();

    if (new_widget != nullptr)
        root_->addWidget(std::move(new_widget));
}


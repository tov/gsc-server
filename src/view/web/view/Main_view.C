#include "../../../Navigate.h"
#include "../../game/HangmanWidget.h"
#include "../../game/HighScoresWidget.h"
#include "../widget/Auth_widget.h"
#include "Admin_view.h"
#include "Main_view.h"

#include <Wt/Auth/AuthWidget.h>
#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WLineEdit.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

using namespace Wt;

Main_view::Main_view(Session &session) : session_(session) {
  head_ = addNew<WTemplate>("<h1>${home} <small>${title}</small></h1>");
  root_ = addNew<WContainerWidget>();

  auto home = head_->bindNew<WText>("home", "gsc");
  home->clicked().connect(Navigate("/"));
  home->setToolTip("Go home");
  home->setStyleClass("home-link");

  set_title("Homework Server");

#if defined(GSC_AUTH_OPEN_AM) || defined(GSC_AUTH_DEBUG)
  addNew<Auth_widget>(session_);
#elif defined(GSC_AUTH_PASSWORD)
  auto authModel =
      std::make_unique<Auth::AuthModel>(Session::auth(), session_.users());
  authModel->addPasswordAuth(&Session::passwordAuth());

  auto authWidget = addNew<Auth::AuthWidget>(session_.login());
  authWidget->setModel(std::move(authModel));
  authWidget->setRegistrationEnabled(false);
  authWidget->processEnvironment();
#endif
}

void Main_view::set_title(const WString &title) {
  set_title(std::make_unique<Wt::WText>(title));
}

void Main_view::set_title(std::unique_ptr<Wt::WWidget> new_title) {
  head_->bindWidget("title", std::move(new_title));
}

void Main_view::set_widget(std::unique_ptr<WWidget> new_widget) {
  root_->clear();

  if (new_widget != nullptr)
    root_->addWidget(std::move(new_widget));
}

#pragma once

#include "Session.h"
#include "common/Media_type_registry.h"

#include <Wt/Dbo/SqlConnectionPool.h>
#include <Wt/Dbo/ptr.h>
#include <Wt/WApplication.h>

#include <memory>
#include <regex>

class Main_view;

class User;
class Assignment;
class Eval_item;
class PageTitle;
class Submission;

class Application_controller : public Wt::WApplication {
public:
  static std::unique_ptr<Wt::WApplication> create(Wt::Dbo::SqlConnectionPool &,
                                                  const Wt::WEnvironment &);

  Application_controller(Wt::Dbo::SqlConnectionPool &,
                         const Wt::WEnvironment &);

private:
  Session session_;
  Main_view *main_;

  void set_page_title(PageTitle &&title);
  void set_page_title(std::string const &text);
  void set_widget(std::unique_ptr<Wt::WWidget>);

  template <class T, class... Arg> void set_new_widget(Arg &&... arg) {
    set_widget(std::make_unique<T>(std::forward<Arg>(arg)...));
  }

  void handle_internal_path(std::string);
  void on_auth_event();

  using smatch = std::smatch;
  using ssubmatch = std::ssub_match;

  Wt::Dbo::ptr<User> find_user(ssubmatch const &,
                               Wt::Dbo::ptr<User> const &current = nullptr);
  Wt::Dbo::ptr<Assignment> find_assignment(const char *);
  int find_eval_item(const dbo::ptr<Assignment> &, const char *);
};

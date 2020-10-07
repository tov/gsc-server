#pragma once

#include "../../../Session.h"
#include "File_manager_view.h"

#include <Wt/WContainerWidget.h>

class Named_tab_widget;

class Session;

class Main_view : public Wt::WContainerWidget {
public:
  explicit Main_view(Session &);

  void set_title(const Wt::WString &);
  void set_title(std::unique_ptr<Wt::WWidget>);

  void set_widget(std::unique_ptr<Wt::WWidget>);

private:
  Wt::WTemplate *head_;
  Wt::WContainerWidget *root_;
  Session &session_;
};

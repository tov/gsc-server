#pragma once

#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WString.h>
#include <Wt/WWidget.h>

class Accelerator_text : public Wt::WCompositeWidget {
public:
  Accelerator_text(const Wt::WString &);

  void set_target(Wt::WWidget *);
  void set_target(Wt::Signal<> *);

  static Wt::WString parse_label(const Wt::WString &, char &key);

private:
  Wt::WWidget *focus_target_ = nullptr;
  Wt::Signal<> *signal_target_ = nullptr;
  char key_;

  void key_press_(const Wt::WKeyEvent &);
  void click_();
};
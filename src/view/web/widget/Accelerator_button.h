#pragma once

#include "Accelerator_text.h"

#include <Wt/WCompositeWidget.h>

class Accelerator_button : public Wt::WCompositeWidget {
public:
  Accelerator_button(const Wt::WString &);

  Wt::Signal<> &clicked() { return clicked_; }

private:
  Wt::Signal<> clicked_;
  char key_;

  void key_press_(const Wt::WKeyEvent &);
  void focused_key_press_(const Wt::WKeyEvent &);
  void click_();
};

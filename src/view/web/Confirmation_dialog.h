#pragma once

#include <Wt/WDialog.h>
#include <Wt/WSignal.h>
#include <Wt/WString.h>

#include <string>

class Confirmation_dialog : public Wt::WDialog {
public:
  static Confirmation_dialog &create(Wt::WString const &);

  template <typename T> static Confirmation_dialog &create(T const &);

  Wt::Signal<> &accepted() { return accepted_; }
  Wt::Signal<> &rejected() { return rejected_; }

private:
  Wt::Signal<> accepted_;
  Wt::Signal<> rejected_;

  explicit Confirmation_dialog(Wt::WString const &);

  void finished_(Wt::DialogCode);
};

template <typename T>
Confirmation_dialog &Confirmation_dialog::create(T const &message) {
  return create(Wt::WString(message));
}

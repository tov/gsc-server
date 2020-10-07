#include "Accelerator_button.h"

#include <Wt/WApplication.h>
#include <Wt/WText.h>

Accelerator_button::Accelerator_button(const Wt::WString &text) {
  auto impl = setNewImplementation<Wt::WText>(
      Accelerator_text::parse_label(text, key_));

  setStyleClass("btn");

  impl->setCanReceiveFocus(true);
  impl->clicked().connect(this, &Accelerator_button::click_);
  impl->enterPressed().connect(this, &Accelerator_button::click_);
  impl->keyPressed().connect(this, &Accelerator_button::focused_key_press_);
  Wt::WApplication::instance()->globalKeyPressed().connect(
      this, &Accelerator_button::key_press_);
}

void Accelerator_button::key_press_(const Wt::WKeyEvent &e) {
  if (std::tolower(e.charCode()) == std::tolower(key_))
    clicked_.emit();
}

void Accelerator_button::focused_key_press_(const Wt::WKeyEvent &e) {
  if (e.charCode() == ' ')
    clicked_.emit();
}

void Accelerator_button::click_() { clicked_.emit(); }

#include "Accelerator_text.h"

#include <Wt/WApplication.h>
#include <Wt/WText.h>

#include <cctype>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

Accelerator_text::Accelerator_text(const Wt::WString &text) {
  auto impl = setNewImplementation<Wt::WText>(parse_label(text, key_));

  impl->clicked().connect(this, &Accelerator_text::click_);
  Wt::WApplication::instance()->globalKeyPressed().connect(
      this, &Accelerator_text::key_press_);
}

void Accelerator_text::set_target(Wt::WWidget *widget) {
  focus_target_ = widget;
}

void Accelerator_text::set_target(Wt::Signal<> *signal) {
  signal_target_ = signal;
}

void Accelerator_text::key_press_(const Wt::WKeyEvent &e) {
  if (std::tolower(e.charCode()) == std::tolower(key_)) {
    if (focus_target_)
      focus_target_->setFocus();
    if (signal_target_)
      signal_target_->emit();
  }
}

void Accelerator_text::click_() {
  if (focus_target_)
    focus_target_->setFocus();
}

namespace {

const std::regex pattern("([^&]*)&[^[:space:]](.*)");

}

Wt::WString Accelerator_text::parse_label(const Wt::WString &text, char &key) {
  std::string label(text.toUTF8());
  std::smatch sm;

  if (!std::regex_match(label, sm, pattern))
    throw std::logic_error("Accelerator_text: '&' required");

  key = sm[1].second[1];

  std::string real_label;
  real_label.reserve(label.size() + sizeof "<u></u>");
  real_label.insert(real_label.end(), sm[1].first, sm[1].second);
  real_label += "<u>";
  real_label.push_back(key);
  real_label += "</u>";
  real_label.insert(real_label.end(), sm[2].first, sm[2].second);

  return Wt::WString::fromUTF8(real_label);
}

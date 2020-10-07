#pragma once

#include "../../../Navigate.h"

#include <Wt/WCompositeWidget.h>
#include <Wt/WSignal.h>

#include <string>

namespace Wt {
class WPushButton;
}

class Glyph_button : public Wt::WCompositeWidget {
public:
  Glyph_button(std::string const &icon, std::string const &title,
               bool icon_only = false);

  Wt::WString styleClass() const override;
  void setStyleClass(WT_USTRING const &styleClass) override;
  void setEnabled(bool enabled);

  Wt::Signal<> &clicked() { return clicked_; }

private:
  Wt::WPushButton *impl_;
  Wt::Signal<> clicked_;
};

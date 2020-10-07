#include "Glyph_button.h"

#include <Wt/WPushButton.h>

Glyph_button::Glyph_button(std::string const &icon, std::string const &title,
                           bool icon_only) {
  auto os = std::ostringstream();
  os << "<span class=\"glyphicon glyphicon-" << icon
     << "\" aria-hidden=\"true\"></span>";
  if (!icon_only)
    os << " " << title;

  impl_ = setNewImplementation<Wt::WPushButton>();
  impl_->setTextFormat(Wt::TextFormat::XHTML);
  impl_->setText(os.str());

  if (icon_only) {
    impl_->setToolTip(title);
    impl_->setAttributeValue("aria-label", title);
  }

  impl_->clicked().connect([=]() { clicked_.emit(); });
}

Wt::WString Glyph_button::styleClass() const {
  return WCompositeWidget::styleClass();
}

void Glyph_button::setStyleClass(WT_USTRING const &styleClass) {
  WCompositeWidget::setStyleClass(styleClass);
}

void Glyph_button::setEnabled(bool enabled) { impl_->setEnabled(enabled); }

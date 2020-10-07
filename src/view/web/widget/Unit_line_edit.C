#include "Unit_line_edit.h"

#include <iomanip>
#include <sstream>

Unit_line_edit::Unit_line_edit() {
  edit_ = setNewImplementation<Wt::WLineEdit>();
  edit_->setPlaceholderText("[0.0, 1.0]");
  edit_->keyWentUp().connect(this, &Unit_line_edit::handle_change_);

  cached_value_ = INVALID;
}

double Unit_line_edit::value() const { return cached_value_; }

void Unit_line_edit::set_value(double numeric) {
  std::ostringstream fmt;
  if (0 <= numeric && numeric <= 1) {
    fmt.setf(std::ios::fixed);
    fmt << std::setprecision(2) << numeric;
    cached_value_ = numeric;
  }
  edit_->setText(fmt.str());
}

void Unit_line_edit::handle_change_() {
  std::istringstream input(edit_->text().toUTF8());

  if (input >> cached_value_ && 0 <= cached_value_ && cached_value_ <= 1) {
    valid().emit();
  } else {
    cached_value_ = INVALID;
    invalid().emit();
  }
}

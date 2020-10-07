#pragma once

#include <Wt/WRegExpValidator.h>
#include <string>

struct Username_validator final : Wt::WRegExpValidator {
  explicit Username_validator(bool strict = false);
  std::string inputFilter() const override;
};

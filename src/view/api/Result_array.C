#include "Result_array.h"

#include <Wt/Json/Object.h>
#include <Wt/Json/Value.h>

namespace J = Wt::Json;

namespace api {

Result_proxy::Result_proxy(Result_array &array, std::string const &status)
    : status_{status}, array_{&array} {}

Result_proxy::~Result_proxy() noexcept(false) {
  J::Object object;
  object[status_] = J::Value(message_.str());
  array_->push_back(std::move(object));
}

Result_proxy Result_array::success() { return Result_proxy{*this, "success"}; }

Result_proxy Result_array::failure() { return Result_proxy{*this, "failure"}; }

void Result_array::add_nested(Result_array nested) {
  J::Object object;
  object["nested"] = J::Value(std::move(nested));
  push_back(std::move(object));
}

} // namespace api
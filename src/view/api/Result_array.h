#pragma once

#include <Wt/Json/Array.h>

#include <sstream>
#include <string>

namespace api {

class Result_array;

class Result_proxy {
public:
  ~Result_proxy() noexcept(false);

  template <class T> Result_proxy &operator<<(T const &arg) {
    message_ << arg;
    return *this;
  }

  Result_proxy(Result_proxy &&) = default;
  Result_proxy &operator=(Result_proxy &&) = default;

  Result_proxy(Result_proxy const &) = delete;
  Result_proxy &operator=(Result_proxy const &) = delete;

private:
  Result_proxy(Result_array &array, std::string const &status);

  std::string status_;
  Result_array *array_;
  std::ostringstream message_;

  friend Result_array;
};

class Result_array : public Wt::Json::Array {
public:
  Result_array() = default;

  Result_proxy success();
  Result_proxy failure();
  void add_nested(Result_array nested);
};

} // namespace api

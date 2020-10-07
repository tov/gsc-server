#pragma once

#include <array>
#include <string>
#include <type_traits>
#include <vector>

template <class T> struct Enum;

template <class T> char const *stringify(T value) {
  return Enum<T>::show(value);
}

class destringify_result {
public:
  friend auto destringify(char const *);

  template <class T> operator T() const { return Enum<T>::read(c_str_); }

private:
  explicit destringify_result(char const *c_str) : c_str_{c_str} {}

  const char *c_str_;
};

inline auto destringify(char const *value) { return destringify_result(value); }

inline auto destringify(std::string const &value) {
  return destringify(value.c_str());
}

template <class T> struct Enumerator_info {
  using enum_t = T;
  using repr_t = std::underlying_type_t<enum_t>;

  enum_t value;
  repr_t repr;
  char const *name;

  Enumerator_info(enum_t value, repr_t repr, char const *name) noexcept
      : value{value}, repr{repr}, name{name} {}

  Enumerator_info(enum_t value, char const *name) noexcept
      : Enumerator_info{value, static_cast<repr_t>(value), name} {}
};

template <class T, size_t n>
using Enum_info = std::array<Enumerator_info<T>, n>;

#pragma once

#include "guard.h"

#include <Wt/WDateTime.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WString.h>

#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>

double clean_grade(double numerator, double denominator = 1.0);

template <class T> struct JSON_traits;

template <> struct JSON_traits<Wt::WDateTime> {
  static Wt::WString format(Wt::WDateTime const &);
  static bool parse(Wt::WDateTime &, Wt::WString const &);
};

template <class T> struct JSON {
  static Wt::WString format(T const &);

  static bool parse(T &, Wt::WString const &);
  static bool parse(T &, std::string const &);
  static bool parse(T &, std::string &&);
  static bool parse(T &, std::string_view);
};

template <class T> Wt::WString json_format(const T &val) {
  return JSON<T>::format(val);
}

template <class T, class U> bool json_parse(T &dst, U &&src) {
  return JSON<T>::parse(dst, std::forward<U>(src));
}

template <class T> Wt::WString JSON<T>::format(const T &val) {
  return JSON_traits<T>::format(val);
}

template <class T> bool JSON<T>::parse(T &out, const Wt::WString &str) {
  return JSON_traits<T>::parse(out, str);
}

template <class T> bool JSON<T>::parse(T &out, std::string &&str) {
  return parse(out, Wt::WString::fromUTF8(move(str)));
}

template <class T> bool JSON<T>::parse(T &out, const std::string &str) {
  return parse(out, Wt::WString::fromUTF8(str));
}

template <class T> bool JSON<T>::parse(T &out, std::string_view str) {
  return parse(out, std::string(str));
}

///
/// Percentages
///

struct percentage {
  static int constexpr default_precision = 3;

  explicit percentage(double value, int precision = default_precision)
      : value(value), precision(precision) {}

  operator std::string() const;
  operator Wt::WString() const;

  static std::ostream &format(std::ostream &, double ratio,
                              int precision = default_precision);

  double value;
  int precision;
};

std::ostream &operator<<(std::ostream &, percentage);

///
/// Thousands commas
///

using Imbue_guard = Basic_guard<std::ios_base, std::locale, std::locale const &,
                                &std::ios_base::imbue>;

std::locale make_comma_locale(std::locale const &old = std::locale());

template <typename T> struct with_commas {
  explicit with_commas(T const &value) : ptr(&value) {}

  operator std::string() const;
  operator Wt::WString() const;

  static std::ostream &format(std::ostream &, T const &);

  T const *ptr;
};

template <typename T>
std::ostream &format_with_commas(std::ostream &o, T const &value) {
  return with_commas<T>::format(o, value);
}

template <typename T> with_commas(T const &) -> with_commas<T>;

template <typename T>
std::ostream &with_commas<T>::format(std::ostream &o, T const &value) {
  Imbue_guard guard{o, make_comma_locale(o.getloc())};
  o << value;
  return o;
}

template <typename T>
std::ostream &operator<<(std::ostream &o, with_commas<T> wc) {
  return with_commas<T>::format(o, *wc.ptr);
}

template <typename T> with_commas<T>::operator std::string() const {
  static std::locale const comma_locale = make_comma_locale();

  std::ostringstream o;
  o.imbue(comma_locale);
  o << *ptr;
  return o.str();
}

template <typename T> with_commas<T>::operator Wt::WString() const {
  return operator std::string();
}

template <class Fn> struct Group {
  Fn fn;

  explicit Group(Fn fn) : fn{std::move(fn)} {}
};

template <class Fn> Group<Fn> group(Fn fn) { return Group<Fn>{std::move(fn)}; }

template <class Fn>
std::ostream &operator<<(std::ostream &os, Group<Fn> const &group) {
  (void)group.fn(os);
  return os;
}

namespace html {

namespace tag {
constexpr char const li[] = "li";
constexpr char const p[] = "p";
constexpr char const ul[] = "ul";
} // namespace tag

namespace elt {

template <const char *tag, class Body> struct Elt {
  Body body;

  explicit Elt(Body body) : body{std::move(body)} {}
};

template <const char *tag, class Body>
std::ostream &operator<<(std::ostream &os, Elt<tag, Body> element) {
  os << "<" << tag << ">";

  return os << "</" << tag << ">";
}

template <class T> struct li : Elt<tag::li, T> {
  li(T body) : Elt<tag::li, T>::Elt{std::move(body)} {}
};

template <class T> struct p : Elt<tag::p, T> { using Elt<tag::p, T>::Elt; };

template <class T> struct ul : Elt<tag::ul, T> { using Elt<tag::ul, T>::Elt; };

} // namespace elt

struct Escape {
  std::string const &raw;
};

std::ostream &operator<<(std::ostream &, Escape);

struct Filename {
  std::string const &filename;
};

std::ostream &operator<<(std::ostream &, Filename);

} // namespace html

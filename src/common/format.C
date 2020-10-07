#include "format.h"
#include "util.h"

#include <Wt/Date/tz.h>
#include <Wt/WString.h>

#include <cmath>

#define JSON_DATE_FMT "yyyy-MM-dd'T'hh:mm:ss.zzzZ"

double constexpr grade_epsilon = 1E-3;

double clean_grade(double num, double den) {
  if (den < grade_epsilon)
    return 0;

  if (double grade = num / den; std::isnormal(grade) && grade >= grade_epsilon)
    return grade;
  else
    return 0;
}

WString JSON_traits<WDateTime>::format(WDateTime const &datetime) {
  return datetime.toString(JSON_DATE_FMT, false);
}

bool JSON_traits<WDateTime>::parse(WDateTime &out, WString const &str) {
  out = WDateTime::fromString(str, JSON_DATE_FMT);
  return out.isValid();
}

using Precision_guard = Basic_guard<std::ios_base, std::streamsize,
                                    std::streamsize, &std::ios_base::precision>;

std::ostream &percentage::format(std::ostream &o, double ratio, int precision) {
  if (ratio == 1.0)
    return o << "100%";
  if (ratio < grade_epsilon)
    return o << "0%";
  if (std::isnan(ratio))
    return o << "—";

  Precision_guard guard{o, precision};
  return o << 100 * ratio << '%';
}

std::ostream &operator<<(std::ostream &o, percentage pct) {
  return percentage::format(o, pct.value, pct.precision);
}

class comma_numpunct : public std::numpunct<char> {
protected:
  char do_thousands_sep() const override;
  std::string do_grouping() const override;
};

char comma_numpunct::do_thousands_sep() const { return ','; }

std::string comma_numpunct::do_grouping() const { return "\003"; }

std::locale make_comma_locale(std::locale const &old) {
  return std::locale{old, new comma_numpunct};
}

percentage::operator std::string() const {
  std::ostringstream oss;
  oss << *this;
  return oss.str();
}

percentage::operator Wt::WString() const { return operator std::string(); }

namespace html {

std::ostream &operator<<(std::ostream &o, Escape x) {
  for (char c : x.raw) {
    switch (c) {
    case '<':
      o << "&lt;";
      break;
    case '>':
      o << "&gt;";
      break;
    case '&':
      o << "&amp;";
      break;
    case '"':
      o << "&quot;";
      break;
    case '\'':
      o << "&apos;";
      break;
    default:
      o << c;
      break;
    }
  }

  return o;
}

std::ostream &operator<<(std::ostream &o, Filename x) {
  return o << "<code>" << Escape{x.filename} << "</code>";
}

} // namespace html

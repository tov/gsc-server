#pragma once

#include "guard.h"

#include <Wt/WDateTime.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WLocale.h>
#include <Wt/WString.h>

#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

double clean_grade(double numerator, double denominator = 1.0);

void set_time_zone(Wt::WLocale& locale);

Wt::WString json_format(Wt::WDateTime const& datetime);

///
/// Percentages
///

struct percentage
{
    static int constexpr default_precision = 3;

    explicit percentage(double value, int precision = default_precision)
            : value(value), precision(precision)
    { }

    operator std::string() const;
    operator Wt::WString() const;

    static std::ostream&
    format(std::ostream&, double ratio, int precision = default_precision);

    double value;
    int precision;
};

std::ostream& operator<<(std::ostream&, percentage);

///
/// Thousands commas
///

using Imbue_guard = Basic_guard<
        std::ios_base,
        std::locale,
        std::locale const&,
        &std::ios_base::imbue
>;

std::locale make_comma_locale(std::locale const& old = std::locale());

template <typename T>
struct with_commas
{
    explicit with_commas(T const& value) : ptr(&value) { }

    operator std::string() const;
    operator Wt::WString() const;

    static std::ostream&
    format(std::ostream&, T const&);

    T const* ptr;
};

template <typename T>
std::ostream& format_with_commas(std::ostream& o, T const& value)
{
    return with_commas<T>::format(o, value);
}

template<typename T>
with_commas(T const&) -> with_commas<T>;

template <typename T>
std::ostream& with_commas<T>::format(std::ostream& o, T const& value)
{
    Imbue_guard guard{o, make_comma_locale(o.getloc())};
    o << value;
    return o;
}

template <typename T>
std::ostream& operator<<(std::ostream& o, with_commas<T> wc)
{
    return with_commas<T>::format(o, *wc.ptr);
}

template <typename T>
with_commas<T>::operator std::string() const
{
    static std::locale const comma_locale = make_comma_locale();

    std::ostringstream o;
    o.imbue(comma_locale);
    o << *ptr;
    return o.str();
}

template <typename T>
with_commas<T>::operator Wt::WString() const
{
    return operator std::string();
}

namespace html
{

struct Escape
{
    std::string const& raw;
};

std::ostream& operator<<(std::ostream&, Escape);

struct Filename
{
    std::string const& filename;
};

std::ostream& operator<<(std::ostream&, Filename);

}


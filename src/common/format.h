#pragma once

#include <Wt/WDateTime.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WLocale.h>
#include <Wt/WString.h>

#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

double clean_grade(double);

void set_time_zone(Wt::WLocale& locale);

Wt::WString http_format(Wt::WDateTime const& datetime);

Wt::WString json_format(Wt::WDateTime const& datetime);

std::string pct_string(double ratio, int precision = 3);

class comma_numpunct : public std::numpunct<char>
{
protected:
    char do_thousands_sep() const override;
    std::string do_grouping() const override;
};

template <class T>
class with_commas
{
public:
    explicit with_commas(T const& value) : ptr_(&value) { }

    std::ostream& format(std::ostream&) const;

    operator std::string() const;
    operator Wt::WString() const;

private:
    T const* ptr_;
};

template<typename T>
with_commas(T const&) -> with_commas<T>;

template <class T>
std::ostream& operator<<(std::ostream& o, with_commas<T> wc)
{
    return wc.format(o);
}

template <class T>
std::ostream& with_commas<T>::format(std::ostream& o) const
{
    std::locale old_locale = o.getloc();
    std::locale new_locale(old_locale, new comma_numpunct);
    o.imbue(new_locale);
    o << ptr_;
    o.imbue(old_locale);
    return o;
}

template <class T>
with_commas<T>::operator std::string() const
{
    static std::locale comma_locale(std::locale(), new comma_numpunct);

    std::ostringstream o;
    o.imbue(comma_locale);
    o << *ptr_;
    return o.str();
}

template <class T>
with_commas<T>::operator Wt::WString() const
{
    return operator std::string();
}


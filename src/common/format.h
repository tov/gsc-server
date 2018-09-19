#pragma once

#include <Wt/WDateTime.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WLocale.h>
#include <Wt/WString.h>

#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>

void set_time_zone(Wt::WLocale& locale);

Wt::WString http_format(Wt::WDateTime const& datetime);

Wt::WString json_format(Wt::WDateTime const& datetime);

class comma_numpunct : public std::numpunct<char>
{
protected:
    char do_thousands_sep() const override;
    std::string do_grouping() const override;
};

template <class T>
class WithCommas
{
public:
    explicit WithCommas(T const& value) : ptr(&value) { }

    template <class U>
    friend std::ostream& operator<<(std::ostream&, WithCommas<U>);

    operator std::string() const;
    operator Wt::WString() const;

private:
    T const* ptr;
};

template <class T>
std::ostream& operator<<(std::ostream& o, WithCommas<T> wc)
{
    std::locale old_locale = o.getloc();
    std::locale new_locale(old_locale, new comma_numpunct);
    o.imbue(new_locale);
    o << *wc.ptr;
    o.imbue(old_locale);
    return o;
}

template <class T>
WithCommas<T>::operator std::string() const
{
    static std::locale comma_locale(std::locale(), new comma_numpunct);

    std::ostringstream o;
    o.imbue(comma_locale);
    o << *ptr;
    return o.str();
}

template <class T>
WithCommas<T>::operator Wt::WString() const
{
    return operator std::string();
}

template <class T>
WithCommas<T> with_commas(T const& value)
{
    return WithCommas<T>(value);
}

double clean_grade(double);


#pragma once

#include <Wt/Json/Value.h>
#include <Wt/WDateTime.h>

#include <string>

template <class T>
class Enum;

template <class T>
char const* stringify(T value)
{
    return Enum<T>::show(value);
}

template <class T>
T destringify(char const* value)
{
    return Enum<T>::read(value);
}

template <class T>
T destringify(std::string const& value)
{
    return Enum<T>::read(value.c_str());
}

#define HTTP_DATE_FMT "ddd, d MMM yyyy hh:mm:ss 'GMT'"

auto inline http_format(Wt::WDateTime const& datetime)
{
    return datetime.toString(HTTP_DATE_FMT);
}

#define JSON_DATE_FMT "yyyy-MM-dd'T'hh:mm:ss.zzz'Z'"

auto inline json_format(Wt::WDateTime const& datetime)
{
    return Wt::Json::Value(datetime.toString(JSON_DATE_FMT));
}

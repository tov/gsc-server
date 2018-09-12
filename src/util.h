#pragma once

#include <Wt/Date/tz.h>
#include <Wt/Json/Value.h>
#include <Wt/WDateTime.h>
#include <Wt/WLocale.h>

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

#define CLIENT_TIME_ZONE "America/Chicago"

inline auto set_time_zone(Wt::WLocale& locale)
{
    auto time_zone = date::locate_zone(CLIENT_TIME_ZONE);
    locale.setTimeZone(time_zone);
}

#define HTTP_DATE_FMT "ddd, d MMM yyyy hh:mm:ss 'GMT'"

inline auto http_format(Wt::WDateTime const& datetime)
{
    return datetime.toString(HTTP_DATE_FMT);
}

#define JSON_DATE_FMT "yyyy-MM-dd'T'hh:mm:ss.zzzZ"

inline auto json_format(Wt::WDateTime const& datetime)
{
    Wt::WLocalDateTime local = datetime.toLocalTime();
    return Wt::Json::Value(local.toString(JSON_DATE_FMT));
}

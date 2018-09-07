#pragma once

#include <Wt/Json/Value.h>
#include <Wt/WDateTime.h>

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

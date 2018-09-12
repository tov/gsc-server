#include "date_time.h"
#include <Wt/Date/tz.h>
#include <Wt/WString.h>

#define CLIENT_TIME_ZONE "America/Chicago"
#define HTTP_DATE_FMT "ddd, d MMM yyyy hh:mm:ss 'GMT'"
#define JSON_DATE_FMT "yyyy-MM-dd'T'hh:mm:ss.zzzZ"

void set_time_zone(Wt::WLocale& locale)
{
    auto time_zone = date::locate_zone(CLIENT_TIME_ZONE);
    locale.setTimeZone(time_zone);
}

Wt::WString http_format(Wt::WDateTime const& datetime)
{
    return datetime.toString(HTTP_DATE_FMT);
}

Wt::WString json_format(Wt::WDateTime const& datetime)
{
    Wt::WLocalDateTime local = datetime.toLocalTime();
    return local.toString(JSON_DATE_FMT);
}

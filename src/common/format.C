#include "format.h"
#include <Wt/Date/tz.h>
#include <Wt/WString.h>

#include <cmath>

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

std::string pct_string(double ratio, int precision)
{
    if (ratio == 1.0) return "100%";
    if (ratio < 0.0001) return "0%";
    if (std::isnan(ratio)) return "—";

    std::ostringstream fmt;
    fmt << std::setprecision(precision) << 100 * ratio << '%';
    return fmt.str();
}

char comma_numpunct::do_thousands_sep() const
{
    return ',';
}

std::string comma_numpunct::do_grouping() const
{
    return "\003";
}

double clean_grade(double grade) {
    return grade < 0.001? 0 : grade;
}

#pragma once

#include <Wt/WDateTime.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WLocale.h>
#include <Wt/WString.h>

void set_time_zone(Wt::WLocale& locale);

Wt::WString http_format(Wt::WDateTime const& datetime);

Wt::WString json_format(Wt::WDateTime const& datetime);

#pragma once

#include "../specializations.h"

#include <Wt/WDateTime.h>

class Time_stamp
{
    friend Wt::Dbo::sql_value_traits<Time_stamp, void>;

public:
    Time_stamp();
    explicit Time_stamp(std::nullptr_t);

    void touch();

    Wt::WDateTime get() const { return value_; }

    operator Wt::WDateTime() const { return get(); }

private:
    Wt::WDateTime value_;
};

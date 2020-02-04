#pragma once

#include "specializations.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WTime.h>
#include <Wt/Dbo/Field>

class User;

class Lab_section
{
public:
    Lab_section() = default;

private:
    Wt::Dbo::ptr<User> leader_;
    int                day_;    // 0 means Sunday
    Wt::WTime          time_;
    std::string        location_;

public:
    template<class Action>
    void persist(Action& a)
    {
        dbo::hasOne(a, leader_, "leader", dbo::OnDeleteSetNull);
        dbo::field(a, day_, "day");
        dbo::field(a, time_, "time");
        dbo::field(a, location_, "location");
    }
};

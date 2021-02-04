#pragma once

#include "specializations.h"
#include "util/Time_stamp.h"

#include <Wt/Dbo/Types.h>

#include <string>

namespace dbo = Wt::Dbo;

class Test_run;

class Log_file
{
public:

private:
    std::string            name_;
    Wt::Dbo::ptr<Test_run> test_run_;

public:
    template<typename Action>
    void persist(Action& a)
    {
        Wt::Dbo::field(a, name_, "name");
        Wt::Dbo::belongsTo(a, test_run_, "test_run",
                           Wt::Dbo::OnDeleteCascade);
    }
};

DBO_EXTERN_TEMPLATES(Test_run)

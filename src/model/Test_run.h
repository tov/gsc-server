#pragma once

#include "specializations.h"

#include "util/Time_stamp.h"

#include <Wt/Dbo/Types.h>

namespace dbo = Wt::Dbo;

class Submission;
class Log_file;

using Log_files = Wt::Dbo::collection<Wt::Dbo::ptr<Log_file>>;

class Test_run
{
public:

private:
    Wt::Dbo::ptr<Submission> submission_;
    double raw_score_;
    Time_stamp submit_time_;
    Time_stamp test_time_;
    Log_files log_files_;

public:
    template<typename Action>
    void persist(Action& a)
    {
        Wt::Dbo::belongsTo(a, submission_, "submission",
                           Wt::Dbo::OnDeleteCascade);
        Wt::Dbo::field(a, raw_score_, "raw_score");
        Wt::Dbo::field(a, submit_time_, "submit_time");
        Wt::Dbo::field(a, test_time_, "test_time");
        Wt::Dbo::hasMany(a, log_files_, Wt::Dbo::ManyToOne, "test_run");
    }
};

DBO_EXTERN_TEMPLATES(Test_run)

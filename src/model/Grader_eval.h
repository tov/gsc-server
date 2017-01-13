#pragma once

#include "specializations.h"

#include "auth/User.h"

#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/Types>
#include <Wt/WDateTime>

#include <string>

namespace dbo = Wt::Dbo;

class User;
class Self_eval;

class Grader_eval
{
public:
    Grader_eval() {}
    Grader_eval(const dbo::ptr<Self_eval>& self_eval,
                const dbo::ptr<User>& grader);

private:
    dbo::ptr<Self_eval> self_eval_;
    dbo::ptr<User>      grader_;
    std::string         content_;
    double              score_;
    Wt::WDateTime       time_stamp_;
    bool                complete_;
    std::string         permalink_;

public:
    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, self_eval_, "self_eval", dbo::OnDeleteCascade);
        dbo::belongsTo(a, grader_, "grader", dbo::OnDeleteSetNull);
        dbo::field(a, content_, "content");
        dbo::field(a, score_, "score");
        dbo::field(a, time_stamp_, "time_stamp");
        dbo::field(a, complete_, "complete");
        dbo::field(a, permalink_, "permalink", 16);
    }
};

DBO_EXTERN_TEMPLATES(Grader_eval);

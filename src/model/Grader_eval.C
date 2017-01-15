#include "Grader_eval.h"
#include "Self_eval.h"
#include "auth/User.h"

#include <Wt/Dbo/Impl>

DBO_INSTANTIATE_TEMPLATES(Grader_eval);

Grader_eval::Grader_eval(const dbo::ptr<Self_eval>& self_eval,
                         const dbo::ptr<User>& grader)
        : self_eval_(self_eval),
          grader_(grader),
          score_(0.0),
          time_stamp_(Wt::WDateTime::currentDateTime()),
          status_(static_cast<int>(Status::editing))
{
}


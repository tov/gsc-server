#include "Grader_eval.h"
#include "Self_eval.h"

#include <Wt/Dbo/Impl>

DBO_INSTANTIATE_TEMPLATES(Grader_eval);

Grader_eval::Grader_eval(const dbo::ptr<Self_eval>& self_eval)
        : self_eval_(self_eval),
          score_(0.0),
          time_stamp_(Wt::WDateTime::currentDateTime()),
          complete_(false)
{

}

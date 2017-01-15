#include "Grader_eval.h"
#include "Self_eval.h"
#include "Session.h"
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

void Grader_eval::set_explanation(const std::string& explanation)
{
    content_ = explanation;
    touch_();
}

void Grader_eval::set_score(double score)
{
    score_ = score;
    touch_();
}

void Grader_eval::touch_()
{
    time_stamp_ = Wt::WDateTime::currentDateTime();
}

dbo::ptr<Grader_eval> Grader_eval::get_for(
        const dbo::ptr<Self_eval>& self_eval,
        Session& session)
{
    auto result = self_eval->grader_eval();

    if (result)
        return result;
    else
        return session.add(new Grader_eval(self_eval, session.user()));
}

std::string Grader_eval::owner_string(const dbo::ptr<User>& as_seen_by) const
{
    if (grader() == as_seen_by && as_seen_by->role() == User::Role::Student)
        return "Auto";

    return grader()->name();
}

const Wt::Dbo::ptr<Eval_item>& Grader_eval::eval_item() const
{
    return self_eval()->eval_item();
}

const Wt::Dbo::ptr<Submission>& Grader_eval::submission() const
{
    return self_eval()->submission();
}

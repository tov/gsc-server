#pragma once

#include <string>

#include <Wt/WDateTime.h>
#include <Wt/Dbo/Field.h>
#include <Wt/Dbo/ptr.h>

namespace dbo = Wt::Dbo;

class Assignment;
class Eval_item;
class User;
class Self_eval;
class Submission;

class Abstract_evaluation
{
public:
    const std::string& explanation() const { return explanation_; }
    virtual void set_explanation(const std::string&);

    double score() const { return score_; }
    virtual void set_score(double);

    virtual std::string owner_string(
            const dbo::ptr<User>& as_seen_by) const = 0;

    virtual const dbo::ptr<Eval_item>& eval_item() const = 0;
    virtual const dbo::ptr<Submission>& submission() const = 0;

    const dbo::ptr<Assignment>& assignment() const;
    virtual std::string score_string() const;

    virtual ~Abstract_evaluation() = default;

protected:
    virtual void touch_();

    template<typename Action>
    void persist_(Action& a)
    {
        dbo::field(a, explanation_, "explanation");
        dbo::field(a, score_, "score");
        dbo::field(a, time_stamp_, "time_stamp");
    }

private:
    std::string         explanation_;
    double              score_;
    Wt::WDateTime       time_stamp_;
};
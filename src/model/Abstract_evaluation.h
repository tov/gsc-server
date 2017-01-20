#pragma once

#include <string>

#include <Wt/Dbo/ptr>

namespace dbo = Wt::Dbo;

class Assignment;
class Eval_item;
class User;
class Self_eval;
class Submission;

struct Abstract_evaluation
{
    virtual double score() const = 0;
    virtual void set_score(double) = 0;

    virtual const std::string& explanation() const = 0;
    virtual void set_explanation(const std::string&) = 0;

    virtual std::string owner_string(
            const dbo::ptr<User>& as_seen_by) const = 0;

    virtual const dbo::ptr<Eval_item>& eval_item() const = 0;
    virtual const dbo::ptr<Submission>& submission() const = 0;
    const dbo::ptr<Assignment>& assignment() const;
    virtual std::string score_string() const;

    virtual ~Abstract_evaluation() {}
};
#pragma once

#include <string>

#include <Wt/Dbo/ptr>

namespace dbo = Wt::Dbo;

class Self_eval;
class User;

struct Abstract_evaluation
{
    virtual double score() const = 0;
    virtual void set_score(double) = 0;

    virtual const std::string& explanation() const = 0;
    virtual void set_explanation(const std::string&) = 0;

    virtual std::string owner_string(
            const dbo::ptr<User>& as_seen_by) const = 0;

    virtual ~Abstract_evaluation() {}
};
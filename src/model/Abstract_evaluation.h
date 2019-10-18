#pragma once

#include <string>

#include <Wt/Json/Object.h>
#include <Wt/WDateTime.h>
#include <Wt/Dbo/Field.h>
#include <Wt/Dbo/ptr.h>

namespace dbo = Wt::Dbo;

class Assignment;
class Eval_item;
class User;
class Self_eval;
class Submission;
class Viewing_context;

class Abstract_evaluation
{
public:
    explicit Abstract_evaluation(double score = 0)
        : score_(normalize_score(score))
    { }

    const std::string& explanation() const { return explanation_; }
    virtual void set_explanation(const std::string&);

    double score() const { return score_; }
    virtual void set_score(double);

    virtual std::string owner_string(Viewing_context const&) const = 0;
    virtual std::string score_string(Viewing_context const&) const = 0;

    virtual const dbo::ptr<Eval_item>& eval_item() const = 0;
    virtual const dbo::ptr<Submission>& submission() const = 0;

    const dbo::ptr<Assignment>& assignment() const;

    Wt::Json::Object to_json() const;

    virtual ~Abstract_evaluation() = default;


protected:
    virtual void touch_();

    virtual std::string plain_score_string() const;
    static double normalize_score(double score);

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
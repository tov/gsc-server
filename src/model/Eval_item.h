#pragma once

#include "specializations.h"

#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/Types>

#include <iostream>
#include <string>

namespace dbo = Wt::Dbo;

class Assignment;
class Self_eval;

using Self_evals = dbo::collection<dbo::ptr<Self_eval>>;

class Eval_item
{
public:
    enum class Type : int
    {
        Boolean,
        Scale,
        Informational,
    };

    Eval_item() {}
    Eval_item(const dbo::ptr<Assignment>&, int sequence);

    const dbo::ptr<Assignment>& assignment() const { return assignment_; }
    int sequence() const { return sequence_; }
    Type type() const { return static_cast<Type>(type_); }
    const std::string& prompt() const { return prompt_; }
    double relative_value() const { return relative_value_; }
    Self_evals self_evals() const { return self_evals_; }

    void set_type(Type t) { type_ = static_cast<int>(t); }
    void set_prompt(const std::string& p) { prompt_ = p; }
    void set_relative_value(double d) { relative_value_ = d; }

    std::string relative_value_str() const;
    void set_relative_value(const std::string&);

private:
    dbo::ptr<Assignment> assignment_;
    int                  sequence_ = 0;
    int                  type_ = 0;
    std::string          prompt_;
    double               relative_value_ = 1.0;
    Self_evals           self_evals_;

public:
    template<class Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, assignment_, "assignment", dbo::OnDeleteCascade);
        dbo::field(a, sequence_, "sequence");
        dbo::field(a, type_, "type");
        dbo::field(a, prompt_, "prompt");
        dbo::field(a, relative_value_, "relative_value");
        dbo::hasMany(a, self_evals_, dbo::ManyToOne, "eval_item");
    }
};

DBO_EXTERN_TEMPLATES(Eval_item);



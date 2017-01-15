#pragma once

#include <Wt/Dbo/Types>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/WDateTime>

namespace dbo = Wt::Dbo;

class Eval_item;
class Grader_eval;
class Submission;

class Self_eval
{
public:
    Self_eval() {}
    Self_eval(const dbo::ptr<Eval_item>&,
              const dbo::ptr<Submission>&);

    const dbo::ptr<Eval_item>& eval_item() const { return eval_item_; }
    const dbo::ptr<Submission>& submission() const { return submission_; }
    dbo::ptr<Grader_eval> grader_eval() const { return grader_eval_.lock(); }
    const std::string& explanation() const { return explanation_; }
    double score() const { return score_; }
    const std::string& permalink() const { return permalink_; }

    void set_explanation(const std::string&);
    void set_score(double);

    static dbo::ptr<Self_eval> find_by_permalink(dbo::Session&,
                                                 const std::string&);
    static std::string find_ungraded_permalink(dbo::Session&);

private:
    dbo::ptr<Eval_item>        eval_item_;
    dbo::ptr<Submission>       submission_;
    dbo::weak_ptr<Grader_eval> grader_eval_;
    std::string                explanation_;
    double                     score_;
    std::string                permalink_;
    Wt::WDateTime              time_stamp_;

    void touch_();

public:
    static const int permalink_size = 16;

    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, eval_item_, "eval_item", dbo::OnDeleteCascade);
        dbo::belongsTo(a, submission_, "submission", dbo::OnDeleteCascade);
        dbo::hasOne(a, grader_eval_, "self_eval");
        dbo::field(a, explanation_, "explanation");
        dbo::field(a, score_, "score");
        dbo::field(a, permalink_, "permalink", permalink_size);
        dbo::field(a, time_stamp_, "time_stamp");
    }
};

DBO_EXTERN_TEMPLATES(Self_eval);

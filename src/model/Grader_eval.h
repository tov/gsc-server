#pragma once

#include "specializations.h"

#include "auth/User.h"
#include "Abstract_evaluation.h"

#include <Wt/Dbo/WtSqlTraits>
#include <Wt/Dbo/Types>
#include <Wt/WDateTime>

#include <string>

namespace dbo = Wt::Dbo;

class User;
class Self_eval;
class Session;

class Grader_eval : public Abstract_evaluation
{
public:
    Grader_eval() {}
    Grader_eval(const dbo::ptr<Self_eval>& self_eval,
                const dbo::ptr<User>& grader);

    enum class Status : int {
        editing,
        held_back,
        ready,
    };

    Status status() const { return static_cast<Status>(status_); }
    void set_status(Status status) { status_ = static_cast<int>(status); }
    const dbo::ptr<Self_eval>& self_eval() const { return self_eval_; }
    const dbo::ptr<User>& grader() const { return grader_; }
    void set_grader(const dbo::ptr<User>& grader) { grader_ = grader; }
    const std::string& explanation() const override { return content_; }
    void set_explanation(const std::string&) override;
    double score() const override { return score_; }
    void set_score(double) override ;
    std::string score_string() const override;
    std::string owner_string(const dbo::ptr<User>& as_seen_by) const override;
    const Wt::Dbo::ptr<Eval_item>& eval_item() const override;
    const Wt::Dbo::ptr<Submission>& submission() const override;

    static dbo::ptr<Grader_eval>
    get_for(const dbo::ptr<Self_eval>&, Session&);

private:
    dbo::ptr<Self_eval> self_eval_;
    dbo::ptr<User>      grader_;
    std::string         content_;
    double              score_;
    Wt::WDateTime       time_stamp_;
    int                 status_;

    void touch_();

public:
    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, self_eval_, "self_eval", dbo::OnDeleteCascade);
        dbo::belongsTo(a, grader_, "grader", dbo::OnDeleteSetNull);
        dbo::field(a, content_, "content");
        dbo::field(a, score_, "score");
        dbo::field(a, time_stamp_, "time_stamp");
        dbo::field(a, status_, "status");
    }
};

DBO_EXTERN_TEMPLATES(Grader_eval);

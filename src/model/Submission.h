#pragma once

#include "specializations.h"

#include <Wt/Dbo/Types>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/WDateTime>

#include <vector>

namespace dbo = Wt::Dbo;

class Assignment;
class File_meta;
class Self_eval;
class User;

using Self_evals      = dbo::collection<dbo::ptr<Self_eval>>;
using Source_files    = dbo::collection<dbo::ptr<File_meta>>;
using Source_file_vec = std::vector<dbo::ptr<File_meta>>;

class Submission
{
public:
    enum class status
    {
        future,
        open,
        self_eval,
        extended,
        extended_eval,
        closed,
    };

    enum class eval_status { empty, started, complete };

    Submission() {};
    Submission(const dbo::ptr<User>&, const dbo::ptr<Assignment>&);

    Source_file_vec source_files_sorted() const;
    const Source_files& source_files() const { return source_files_; }
    size_t file_count() const;

    const Wt::WDateTime& last_modified() const { return last_modified_; }

    const dbo::ptr<Assignment>& assignment() const { return assignment_; }
    const dbo::ptr<User>& user1() const { return user1_; }
    const dbo::ptr<User>& user2() const { return user2_; }
    const Self_evals& self_evals() const { return self_evals_; }

    bool extended() const;
    bool eval_extended() const;
    const Wt::WDateTime& effective_due_date() const;
    const Wt::WDateTime& effective_eval_date() const;

    void set_user2(const dbo::ptr<User>& user2) { user2_ = user2; }
    void set_due_date(const Wt::WDateTime& date) { due_date_ = date; }
    void set_eval_date(const Wt::WDateTime& date) { eval_date_ = date; }

    status get_status() const;
    eval_status get_eval_status() const;

    bool can_view(const dbo::ptr<User>&) const;
    bool can_submit(const dbo::ptr<User>&) const;
    bool can_eval(const dbo::ptr<User>&) const;

    std::string url(const dbo::ptr<User>&) const;
    std::string eval_url(const dbo::ptr<User>&) const;

    void touch();

    static dbo::ptr<Submission>
    find_by_assignment_and_user(dbo::Session&,
                                const dbo::ptr<Assignment>&,
                                const dbo::ptr<User>&);

private:
    dbo::ptr<User>       user1_;
    dbo::ptr<User>       user2_;
    dbo::ptr<Assignment> assignment_;
    Self_evals           self_evals_;
    Source_files         source_files_;
    Wt::WDateTime        due_date_;
    Wt::WDateTime        eval_date_;
    Wt::WDateTime        last_modified_;

public:
    template<typename Action>
    void persist(Action& a)
    {
        dbo::belongsTo(a, user1_, "user1", dbo::OnDeleteCascade);
        dbo::belongsTo(a, user2_, "user2", dbo::OnDeleteSetNull);
        dbo::belongsTo(a, assignment_, "assignment", dbo::OnDeleteCascade);
        dbo::hasMany(a, self_evals_, dbo::ManyToOne, "submission");
        dbo::hasMany(a, source_files_, dbo::ManyToOne, "submission");
        dbo::field(a, due_date_, "due_date");
        dbo::field(a, eval_date_, "eval_date");
        dbo::field(a, last_modified_, "last_modified");
    }
};

DBO_EXTERN_TEMPLATES(Submission);

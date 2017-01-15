#pragma once

#include "specializations.h"

#include <Wt/Dbo/Types>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/WDateTime>

#include <vector>

namespace dbo = Wt::Dbo;

class Assignment;
class Eval_item;
class File_meta;
class Grader_eval;
class Self_eval;
class User;

using Self_evals      = dbo::collection<dbo::ptr<Self_eval>>;
using Source_files    = dbo::collection<dbo::ptr<File_meta>>;
using Source_file_vec = std::vector<dbo::ptr<File_meta>>;

class Submission
{
public:
    enum class Status
    {
        future,
        open,
        self_eval,
        extended,
        extended_eval,
        closed,
    };

    enum class Eval_status { empty, started, complete };

    struct Item
    {
        dbo::ptr<Eval_item> eval_item;
        dbo::ptr<Self_eval> self_eval;
        dbo::ptr<Grader_eval> grader_eval;
    };

    using Items = std::vector<Item>;

    Submission() {};
    Submission(const dbo::ptr<User>&, const dbo::ptr<Assignment>&);

    Source_file_vec source_files_sorted() const;
    const Source_files& source_files() const { return source_files_; }
    size_t file_count() const;

    const Wt::WDateTime& last_modified() const { return last_modified_; }

    const dbo::ptr<Assignment>& assignment() const { return assignment_; }
    const dbo::ptr<User>& user1() const { return user1_; }
    const dbo::ptr<User>& user2() const { return user2_; }
    std::string owner_string() const;

    size_t item_count() const;
    const Items& items() const;
    double point_value() const;
    bool is_evaluated() const;
    bool is_graded() const;

    void load_cache() const;

    bool extended() const;
    bool eval_extended() const;
    const Wt::WDateTime& effective_due_date() const;
    const Wt::WDateTime& effective_eval_date() const;

    void set_user2(const dbo::ptr<User>& user2) { user2_ = user2; }
    void set_due_date(const Wt::WDateTime& date) { due_date_ = date; }
    void set_eval_date(const Wt::WDateTime& date) { eval_date_ = date; }

    Status status() const;
    Eval_status eval_status() const;

    bool can_view(const dbo::ptr<User>&) const;
    bool can_submit(const dbo::ptr<User>&) const;
    bool can_eval(const dbo::ptr<User>&) const;
    bool can_view_eval(const dbo::ptr<User>&) const;

    std::string url() const;
    std::string eval_url() const;

    static const dbo::ptr<Self_eval>&
    get_self_eval(const dbo::ptr<Eval_item>&,
                  const dbo::ptr<Submission>&);
    static void retract_self_eval(const dbo::ptr<Self_eval>&);
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

    // caching
    mutable bool is_loaded_ = false;
    mutable size_t item_count_;
    mutable Items items_;
    mutable double point_value_;
    mutable Eval_status eval_status_;
    mutable bool is_graded_;

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

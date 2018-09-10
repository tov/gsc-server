#pragma once

#include "specializations.h"

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WDateTime.h>

#include <vector>

namespace dbo = Wt::Dbo;

class Assignment;
class Eval_item;
class File_meta;
class Grader_eval;
class Self_eval;
class Session;
class User;

using Self_evals      = dbo::collection<dbo::ptr<Self_eval>>;
using Source_files    = dbo::collection<dbo::ptr<File_meta>>;
using Source_file_vec = std::vector<dbo::ptr<File_meta>>;

class Submission : public dbo::Dbo<Submission>
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
    int byte_count() const;

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
    const Wt::WDateTime& open_date() const;
    const Wt::WDateTime& effective_due_date() const;
    const Wt::WDateTime& effective_eval_date() const;

    void set_user2(const dbo::ptr<User>& user2) { user2_ = user2; }
    void set_due_date(const Wt::WDateTime& date) { due_date_ = date; }
    void set_eval_date(const Wt::WDateTime& date) { eval_date_ = date; }
    void clear_files();

    Status status() const;
    Eval_status eval_status() const;
    double grade() const;
    std::string grade_string() const;

    static char const* status_to_string(Status);
    static char const* eval_status_to_string(Eval_status);

    bool can_view(const dbo::ptr<User>&) const;
    bool can_submit(const dbo::ptr<User>&) const;
    bool can_eval(const dbo::ptr <User>&) const;
    bool can_view_eval(const dbo::ptr<User>&) const;

    std::string url() const;
    std::string eval_url() const;

    dbo::ptr<File_meta> find_file_by_name(const std::string&) const;

    int remaining_space() const;
    bool has_sufficient_space(int bytes, const std::string& filename) const;

    static dbo::ptr<Self_eval>
    get_self_eval(const dbo::ptr<Eval_item>&,
                  const dbo::ptr<Submission>&);

    static void retract_self_eval(const dbo::ptr<Self_eval>&);

    static void save_self_eval(const dbo::ptr<Self_eval>&, Session&,
                               double score, const std::string& explanation);

    static dbo::ptr<Grader_eval>
    get_grader_eval(const dbo::ptr<Self_eval>&, const dbo::ptr<User>&);

    static void retract_grader_eval(const dbo::ptr<Grader_eval>&);

    static bool join_together(dbo::ptr<Submission> keep,
                              dbo::ptr<Submission> kill);

    void touch();

    static dbo::ptr<Submission>
    find_by_assignment_and_user(dbo::Session&,
                                const dbo::ptr<Assignment>&,
                                const dbo::ptr<User>&);

    static dbo::ptr<Submission>
    find_by_id(dbo::Session&, int session_id);

    static const int max_byte_count;

    std::string rest_uri() const;
    std::string rest_files_uri() const;
    Wt::Json::Object to_json(bool brief = false) const;

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
    mutable double grade_;

    // cached separately
    mutable int byte_count_ = -1;

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

DBO_EXTERN_TEMPLATES(Submission)

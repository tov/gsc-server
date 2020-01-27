#pragma once

#include "specializations.h"
#include "../common/stringify.h"

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>
#include <Wt/WDateTime.h>

#include <array>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

class Assignment;
class Eval_item;
class File_meta;
class Grader_eval;
class Self_eval;
class Session;
class User;

using Self_evals      = Wt::Dbo::collection<Wt::Dbo::ptr<Self_eval>>;
using Self_eval_vec   = std::vector<Wt::Dbo::ptr<Self_eval>>;
using Source_files    = Wt::Dbo::collection<Wt::Dbo::ptr<File_meta>>;
using Source_file_vec = std::vector<Wt::Dbo::ptr<File_meta>>;

class Submission : public Wt::Dbo::Dbo<Submission>
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

    enum class Eval_status { empty, started, overdue, complete };

    enum class Grading_status { incomplete, complete, regrade };

    struct Item
    {
        Wt::Dbo::ptr<Eval_item> eval_item;
        Wt::Dbo::ptr<Self_eval> self_eval;
        Wt::Dbo::ptr<Grader_eval> grader_eval;
    };

    class Join_collision;
    class Move_collision;

    using Items = std::vector<Item>;

    Submission() {};
    Submission(const Wt::Dbo::ptr<User>&, const Wt::Dbo::ptr<Assignment>&);

    Source_file_vec source_files_sorted() const;
    const Source_files& source_files() const { return source_files_; }
    Self_eval_vec self_eval_vec() const;
    size_t file_count() const;
    int byte_count() const;
    int bytes_quota() const { return bytes_quota_; }

    const Wt::WDateTime& last_modified() const { return last_modified_; }

    int assignment_number() const;
    const Wt::Dbo::ptr<Assignment>& assignment() const { return assignment_; }
    const Wt::Dbo::ptr<User>& user1() const { return user1_; }
    const Wt::Dbo::ptr<User>& user2() const { return user2_; }
    std::string owner_string() const;

    size_t item_count() const;
    const Items& items() const;
    double point_value() const;
    bool is_evaluated() const;
    bool is_graded() const;
    Grading_status grading_status() const;

    void load_cache() const;

    bool extended() const;
    bool eval_extended() const;
    const Wt::WDateTime& open_date() const;
    const Wt::WDateTime& effective_due_date() const;
    const Wt::WDateTime& effective_eval_date() const;

    bool in_submit_period() const;
    bool in_eval_period() const;

    void set_user2(const Wt::Dbo::ptr<User>& user2) { user2_ = user2; }
    void set_due_date(const Wt::WDateTime& date) { due_date_ = date; }
    void set_eval_date(const Wt::WDateTime& date) { eval_date_ = date; }
    void set_bytes_quota(int bytes) { bytes_quota_ = bytes; }
    void clear_files();
    bool divorce();

    Status status() const;
    Eval_status eval_status() const;
    double grade() const;
    std::string grade_string() const;

    bool can_view(const Wt::Dbo::ptr<User>&) const;
    bool can_submit(const Wt::Dbo::ptr<User>&) const;
    bool can_eval(const Wt::Dbo::ptr <User>&) const;
    bool can_view_eval(const Wt::Dbo::ptr<User>&) const;

    std::string url(bool eval = false) const;
    std::string eval_url() const;

    std::string url_for_user(const Wt::Dbo::ptr<User>&,
                             bool eval = false) const;

    Wt::Dbo::ptr<Submission> find_this() const;

    Wt::Dbo::ptr<File_meta> find_file_by_name(const std::string&) const;

    int remaining_space() const;
    bool has_sufficient_space(int bytes, const std::string& filename) const;

    static Wt::Dbo::ptr<Self_eval>
    get_self_eval(const Wt::Dbo::ptr<Eval_item>&,
                  const Wt::Dbo::ptr<Submission>&,
                  bool create = true);

    static Wt::Dbo::ptr<Self_eval>
    get_self_eval(int sequence_number,
                  const Wt::Dbo::ptr<Submission>&,
                  bool create = true);

    static void save_self_eval(const Wt::Dbo::ptr<Self_eval>&,
                               Session&,
                               double score,
                               const std::string& explanation);

    static void retract_self_eval(const Wt::Dbo::ptr<Self_eval>&);

    static Wt::Dbo::ptr<Grader_eval>
    get_grader_eval(const Wt::Dbo::ptr<Self_eval>&,
                    const Wt::Dbo::ptr<User>& create_grader = nullptr);

    static Wt::Dbo::ptr<Grader_eval>
    set_grader_eval(const Wt::Dbo::ptr<Self_eval>&,
                    const Wt::Dbo::ptr<User>& grader,
                    double score,
                    const std::string& explanation = "");

    static void retract_grader_eval(const Wt::Dbo::ptr<Grader_eval>&);

    static bool join_together(Wt::Dbo::ptr<Submission> keep,
                              Wt::Dbo::ptr<Submission> kill);

    static std::set<std::string> intersection(Wt::Dbo::ptr<Submission>,
                                              Wt::Dbo::ptr<Submission>);


    void touch();
    void light_touch() const;

    // Creates the submission if it doesn't exist.
    static Wt::Dbo::ptr<Submission>
    find_by_assignment_and_user(Wt::Dbo::Session&,
                                const Wt::Dbo::ptr<Assignment>&,
                                const Wt::Dbo::ptr<User>&);

    // DOESN'T create the submission if it doesn't exist.
    static Wt::Dbo::ptr<Submission>
    find_by_assignment_number_and_user(Wt::Dbo::Session&,
                                       int assignment_number,
                                       const Wt::Dbo::ptr<User>&);

    static Wt::Dbo::ptr<Submission>
    find_by_id(Wt::Dbo::Session&, int session_id);

    std::string rest_uri() const;
    std::string files_rest_uri() const;
    std::string evals_rest_uri() const;
    Wt::Json::Object to_json(bool brief = false) const;

private:
    Wt::Dbo::ptr<User>       user1_;
    Wt::Dbo::ptr<User>       user2_;
    Wt::Dbo::ptr<Assignment> assignment_;
    Self_evals           self_evals_;
    Source_files         source_files_;
    Wt::WDateTime        due_date_;
    Wt::WDateTime        eval_date_;
    Wt::WDateTime        last_modified_;
    int                  bytes_quota_;

    // caching
    mutable bool is_loaded_ = false;
    mutable size_t item_count_;
    mutable Items items_;
    mutable double point_value_;
    mutable Eval_status eval_status_;
    mutable Grading_status grading_status_;
    mutable double grade_;

    // cached separately
    mutable int byte_count_ = -1;

    static Wt::Dbo::ptr<Grader_eval>&
    find_grader_eval_(const Wt::Dbo::ptr<Self_eval>&);

    Eval_status eval_status_given_counts_(size_t self_eval_count,
                                          size_t eval_item_count) const;

    Grading_status grading_status_given_counts_(size_t grader_eval_count,
                                                size_t self_eval_count) const;

public:
    template<typename Action>
    void persist(Action& a)
    {
        Wt::Dbo::belongsTo(a, user1_, "user1", Wt::Dbo::OnDeleteCascade);
        Wt::Dbo::belongsTo(a, user2_, "user2", Wt::Dbo::OnDeleteSetNull);
        Wt::Dbo::belongsTo(a, assignment_, "assignment", Wt::Dbo::OnDeleteCascade);
        Wt::Dbo::hasMany(a, self_evals_, Wt::Dbo::ManyToOne, "submission");
        Wt::Dbo::hasMany(a, source_files_, Wt::Dbo::ManyToOne, "submission");
        Wt::Dbo::field(a, due_date_, "due_date");
        Wt::Dbo::field(a, eval_date_, "eval_date");
        Wt::Dbo::field(a, last_modified_, "last_modified");
        Wt::Dbo::field(a, bytes_quota_, "bytes_quota");
    }
};

class Submission::Join_collision : public std::runtime_error {
public:
    using submission_t = Wt::Dbo::ptr<Submission>;
    using filenames_t  = std::set<std::string>;

    Join_collision(submission_t, submission_t, filenames_t);

    filenames_t const& filenames() const { return filenames_; }

    std::ostream& write_html(std::ostream&) const;

private:
    std::array<submission_t, 2> submissions_;
    filenames_t filenames_;
};

class Submission::Move_collision : public std::runtime_error {
public:
    using submission_t = Wt::Dbo::ptr<Submission>;
    using filename_t = std::string;

    Move_collision(submission_t, filename_t, submission_t, filename_t);

private:
    std::array<submission_t, 2> submissions_;
    std::array<filename_t, 2> filenames_;
};

struct Viewing_context
{
    Wt::Dbo::ptr<User> viewer;
};

template <>
struct Enum<Submission::Status>
{
    static char const* show(Submission::Status);
};

template <>
struct Enum<Submission::Eval_status>
{
    static char const* show(Submission::Eval_status);
};

DBO_EXTERN_TEMPLATES(Submission)

#include "Submission.h"
#include "Self_eval.h"
#include "Assignment.h"
#include "Eval_item.h"
#include "File_data.h"
#include "File_meta.h"
#include "Grader_eval.h"
#include "auth/User.h"
#include "../Config.h"
#include "../Session.h"
#include "../common/format.h"
#include "../common/util.h"
#include "../common/paths.h"

#include <Wt/WDateTime.h>
#include <Wt/Dbo/Impl.h>
#include <Wt/Dbo/ptr.h>

#include <algorithm>
#include <functional>
#include <locale>
#include <sstream>

DBO_INSTANTIATE_TEMPLATES(Submission)

static int const initial_bytes_quota = 20 * 1024 * 1024;

Wt::WDateTime now_()
{
    return Wt::WDateTime::currentDateTime();
}

Submission::Submission(const dbo::ptr<User>& user,
                       const dbo::ptr<Assignment>& assignment)
        : user1_(user)
        , assignment_(assignment)
        , due_date_(assignment->due_date())
        , eval_date_(assignment->eval_date())
        , last_modified_(now_())
        , bytes_quota_(initial_bytes_quota)
{ }

Source_file_vec Submission::source_files_sorted() const
{
    Source_file_vec result(source_files().begin(),
                           source_files().end());

    std::sort(result.begin(), result.end(),
              [=](auto const& a, auto const& b) { return *a < *b; });

    return result;
}

size_t Submission::file_count() const
{
    return source_files().size();
}

int Submission::byte_count() const
{
    if (byte_count_ == -1) {
        byte_count_ = 0;

        for (const auto& meta : source_files()) {
            if (!meta->uploader()->can_admin()) {
                byte_count_ += meta->byte_count();
            }
        }
    }

    return byte_count_;
}

bool Submission::extended() const
{
    return due_date_ > assignment_->due_date();
}

bool Submission::eval_extended() const
{
    return eval_date_ > assignment_->eval_date();
}

const Wt::WDateTime& Submission::open_date() const
{
    return assignment()->open_date();
}

const Wt::WDateTime& Submission::effective_due_date() const
{
    return std::max(due_date_, assignment_->due_date());
}

const Wt::WDateTime& Submission::effective_eval_date() const
{
    return std::max(eval_date_, assignment_->eval_date());
}

bool Submission::in_submit_period() const
{
    auto stat = status();
    return stat == Status::open
           || stat == Status::extended
           || stat == Status::overtime;
}

bool Submission::in_eval_period() const
{
    auto stat = status();
    return stat == Status::overtime
           || stat == Status::self_eval
           || stat == Status::extended_eval;
}

Submission::Status Submission::status() const
{
    auto overtime_or = [=](Status status) {
        return has_started_self_eval()
               ? status
               : Status::overtime;
    };

    auto now = now_();

    if (now <= assignment()->open_date()) {
        return Status::future;
    } else if (now <= assignment()->due_date()) {
        return Status::open;
    } else if (now <= due_date_) {
        return Status::extended;
    } else if (now <= assignment()->eval_date()) {
        return overtime_or(Status::self_eval);
    } else if (now <= eval_date_) {
        return overtime_or(Status::extended_eval);
    } else {
        return Status::closed;
    }
}

Submission::Eval_status
Submission::eval_status_given_counts_(Eval_counts counts) const
{
    if (counts.self == counts.item) {
        return Eval_status::complete;
    } else if (status() == Status::closed) {
        return Eval_status::overdue;
    } else if (has_started_self_eval()) {
        return Eval_status::started;
    } else {
        return Eval_status::empty;
    }
}

bool
Submission::is_ready_given_counts_(Eval_counts counts) const
{
    // It isn't ready if there are no evaluation items.
    if (counts.item == 0) {
        return false;
    }

    switch (eval_status_given_counts_(counts)) {
    case Eval_status::complete:
    case Eval_status::overdue:
        return counts.grader == counts.self;

    case Eval_status::started:
    case Eval_status::empty:
        return false;
    }
}

Submission::Eval_status Submission::eval_status() const
{
    if (is_loaded_) {
        return eval_status_;
    }

    Eval_counts counts {
        .item      = assignment()->eval_items().size(),
        .self      = self_evals_.size(),
        .grader    = 0,
    };
    return eval_status_given_counts_(counts);
}

double Submission::grade() const
{
    if (is_loaded_) {
        return grade_;
    }

    const char* const sql_query =
            "SELECT"
            " (SELECT SUM(g.score * e.relative_value)"
            "    FROM grader_eval g"
            "   INNER JOIN self_eval s ON s.id = g.self_eval_id"
            "   INNER JOIN eval_item e ON e.id = s.eval_item_id"
            "   WHERE s.submission_id = ?)"
            " /"
            " (SELECT NULLIF(SUM(relative_value), 0)"
            "    FROM eval_item"
            "   WHERE assignment_number = ?)";

    double grade_value = session()->query<double>(sql_query)
                                  .bind(id()).bind(assignment_number())
                                  .resultValue();

    return clean_grade(grade_value);
}

bool Submission::has_started_self_eval() const
{
    return has_self_eval_number(1);
}

bool Submission::has_self_eval_number(int sequence_number) const
{
    dbo::Transaction trans(*session());

    const char* const sql_query =
            "SELECT COUNT(*)"
            "  FROM self_eval s"
            " INNER JOIN eval_item e ON e.id = s.eval_item_id"
            " WHERE s.submission_id = ?"
            "   AND e.sequence = ?";

    return 0 < session()->query<int>(sql_query)
                        .bind(id())
                        .bind(sequence_number)
                        .resultValue();
}

dbo::ptr<Self_eval>
Submission::get_self_eval(const dbo::ptr<Eval_item>& eval_item,
                          const dbo::ptr<Submission>& submission,
                          bool create)
{
    submission->load_cache();

    auto& result = submission->items_[eval_item->sequence()].self_eval;

    if (!result && create) {
        result = Self_eval::create(eval_item, submission);
    }

    return result;
}

dbo::ptr<Self_eval>
Submission::get_self_eval(int sequence,
                          const dbo::ptr<Submission>& submission,
                          bool create)
{
    submission->load_cache();
    auto eval_item = submission->items_.at(sequence).eval_item;
    return get_self_eval(eval_item, submission, create);
}

void Submission::retract_self_eval(const dbo::ptr<Self_eval>& self_eval)
{
    auto submission = self_eval->submission();
    auto sequence   = self_eval->eval_item()->sequence();

    submission->load_cache();

    submission->items_[sequence].grader_eval.remove();
    submission->items_[sequence].grader_eval = {};
    submission->items_[sequence].self_eval.remove();
    submission->items_[sequence].self_eval = {};

    submission.modify()->touch();
}

void
Submission::save_self_eval(const Wt::Dbo::ptr<Self_eval>& self_eval,
                           const Wt::Dbo::ptr<User>& user, double score,
                           const std::string& explanation)
{
    if (self_eval->fully_frozen()) return;

    bool frozen_score = self_eval->score_frozen();

    {
        auto self_eval_m = self_eval.modify();
        if (!frozen_score) self_eval_m->set_score(score);
        self_eval_m->set_explanation(explanation);
        score = self_eval_m->score();
    }

    auto submission  = self_eval->submission();
    auto eval_item   = self_eval->eval_item();
    auto sequence    = eval_item->sequence();
    auto grader_eval = submission->find_grader_eval_(sequence);

    if (score == 0.0 && eval_item->type() == Eval_item::Type::Boolean) {
        set_grader_eval(self_eval, user, CONFIG().reward_for_no(), "");
    } else if (grader_eval && !eval_item->is_informational()) {
        if (frozen_score)
            submission->hold_grader_eval(sequence);
        else
            submission->retract_grader_eval(sequence);
    }

    self_eval->submission().modify()->touch();
}

dbo::ptr<Grader_eval>& Submission::find_grader_eval_(int sequence) const
{
    load_cache();
    return items_.at(sequence).grader_eval;
}

dbo::ptr<Grader_eval>&
Submission::find_grader_eval_(const dbo::ptr<Eval_item>& eval_item) const
{
    return find_grader_eval_(eval_item->sequence());
}

dbo::ptr<Grader_eval>&
Submission::find_grader_eval_(const dbo::ptr<Self_eval>& self_eval)
{
    return self_eval->submission()
                    ->find_grader_eval_(self_eval->eval_item());
}

dbo::ptr<Grader_eval>
Submission::get_grader_eval(const dbo::ptr<Self_eval>& self_eval,
                            const dbo::ptr<User>& grader)
{
    auto& result = find_grader_eval_(self_eval);

    if (!result && grader) {
        result = self_eval.session()->addNew<Grader_eval>(self_eval, grader);
        self_eval->submission().modify()->touch();
    }

    return result;
}

dbo::ptr<Grader_eval>
Submission::set_grader_eval(const dbo::ptr<Self_eval>& self_eval,
                            const dbo::ptr<User>& grader,
                            double score,
                            std::string const& explanation)
{
    auto& result = find_grader_eval_(self_eval);

    if (result) {
        auto result_m = result.modify();
        result_m->set_grader(grader);
        result_m->set_score(score);
        result_m->set_explanation(explanation);
    } else {
        result = self_eval.session()->addNew<Grader_eval>(
                self_eval, grader, score, explanation);
    }

    self_eval->submission().modify()->touch();

    return result;
}

void Submission::hold_grader_eval(int sequence) const
{
    load_cache();
    auto& item = items_.at(sequence);

    if (!item.eval_item->is_graded_automatically()) {
        auto grader_eval_m = item.grader_eval.modify();
        grader_eval_m->set_status(Grader_eval::Status::held_back);
    }
}

void Submission::regrade_grader_eval(int sequence) const
{
    load_cache();
    auto& item = items_.at(sequence);

    if (!item.eval_item->is_graded_automatically()) {
        auto grader_eval_m = item.grader_eval.modify();
        grader_eval_m->set_status(Grader_eval::Status::regrade);
    }
}

void Submission::retract_grader_eval(int sequence) const
{
    load_cache();
    auto& item = items_.at(sequence);

    if (!item.eval_item->is_graded_automatically()) {
        item.grader_eval.remove();
        item.grader_eval = {};
    }
}

void Submission::retract_grader_eval(const dbo::ptr<Grader_eval>& grader_eval)
{
    auto submission = grader_eval->submission();
    auto sequence   = grader_eval->eval_item()->sequence();
    submission->retract_grader_eval(sequence);
    submission.modify()->touch();
}

void Submission::touch()
{
    last_modified_ = now_();
    light_touch();
}

void Submission::light_touch() const
{
    clear_cache();
}

dbo::ptr<Submission>
Submission::find_by_assignment_and_user(dbo::Session& session,
                                        const dbo::ptr<Assignment>& assignment,
                                        const dbo::ptr<User>& user)
{
    if (dbo::ptr<Submission> result = find_by_assignment_number_and_user(
            session, assignment->number(), user)) {
        return result;
    } else {
        return session.addNew<Submission>(user, assignment);
    }
}

dbo::ptr<Submission>
Submission::find_by_assignment_number_and_user(dbo::Session& session,
                                               int assignment_number,
                                               const dbo::ptr<User>& user)
{
    return session.find<Submission>()
                  .where("user1_id = ? OR user2_id = ?")
                  .bind(user.id()).bind(user.id())
                  .where("assignment_number = ?")
                  .bind(assignment_number);
}

dbo::ptr<Submission> Submission::find_by_id(dbo::Session& session,
                                            int submission_id)
{
    return session.find<Submission>()
                  .where("id = ?").bind(submission_id);
}

bool Submission::can_view(const dbo::ptr<User>& user) const
{
    if (user->can_admin()) return true;

    return user == user1_ || user == user2_;
}
bool Submission::can_submit(const dbo::ptr<User>& user) const
{
    if (user->can_admin()) return true;

    return can_view(user) && in_submit_period();
}

bool Submission::can_eval(const dbo::ptr<User>& user) const
{
    if (user->can_admin()) return true;
    if (user != user1_ && user != user2_) return false;

    return in_eval_period();
}

bool Submission::can_view_eval(const dbo::ptr<User>& user) const
{
    return user->can_admin() ||
           (effective_due_date() < now_() &&
            (user == user1_ || user == user2_));
}

void Submission::check_can_view(const dbo::ptr<User>& user) const
{
    if (user->can_admin()) return;

    if (!can_view(user))
        throw Access_check_failed("That isn’t yours.");
}

void Submission::check_can_submit(const dbo::ptr<User>& user) const
{
    if (user->can_admin()) return;

    check_can_view(user);

    if (!in_submit_period())
        throw Access_check_failed("Submission is closed.");
}

void Submission::check_can_eval(const dbo::ptr<User>& user) const
{
    if (user->can_admin()) return;

    check_can_view(user);

    if (!in_eval_period())
        throw Access_check_failed("Self evaluation is closed.");
}

void Submission::check_can_view_eval(const dbo::ptr<User>& user) const
{
    if (user->can_admin()) return;

    check_can_view(user);

    if (effective_due_date() >= now_())
        throw Access_check_failed("Too early for that!");
}

std::string Submission::eval_url() const
{
    return eval_url_for_user(user1_);
}

std::string Submission::files_url() const
{
    return files_url_for_user(user1_);
}

std::string
Submission::eval_url_for_user(const dbo::ptr<User>& principal) const
{
    return url_for_user_(principal, "/eval");
}

std::string
Submission::files_url_for_user(const dbo::ptr<User>& principal) const
{
    return url_for_user_(principal, "");
}

std::string
Submission::url_for_user_(const dbo::ptr<User>& principal,
                         std::string extra_path)
const
{
    std::ostringstream result;

    auto const& owner =
            (user2_ && user2_->id() == principal->id())
            ? user2_
            : user1_;

    result << "/~" << owner->name();
    result << "/hw/" << assignment_->number();
    result << extra_path;

    return result.str();
}

size_t Submission::item_count() const
{
    if (is_loaded_) return item_count_;
    return assignment()->eval_items().size();
}

const std::vector<Submission::Item>& Submission::items() const
{
    load_cache();
    return items_;
}

double Submission::point_value() const
{
    if (is_loaded_) return point_value_;

    return assignment()->total_relative_value();
}

bool Submission::is_evaluated() const
{
    return eval_status() == Eval_status::complete;
}

bool Submission::is_ready() const
{
    if (is_loaded_) {
        return is_ready_;
    }

    Eval_counts counts {
        .item      = assignment()->eval_items().size(),
        .self      = self_evals_.size(),
        .grader    = fetch_grader_eval_count_(),
    };

    return is_ready_given_counts_(counts);
}

size_t Submission::fetch_real_self_eval_count_() const
{
    return session()->query<int>(
            "SELECT COUNT(*)"
            "  FROM self_eval"
            " INNER JOIN eval_item ON eval_item_id = eval_item.id"
            " WHERE submission_id = ?"
            "   AND type <> ?")
        .bind(id())
        .bind(Eval_item::Type::Informational)
        .resultValue();
}

size_t Submission::fetch_grader_eval_count_() const
{
    return session()->query<int>(
            "SELECT COUNT(*)"
            "  FROM self_eval s"
            " INNER JOIN grader_eval g ON g.self_eval_id = s.id"
            " WHERE s.submission_id = ?"
            "   AND g.status = ?")
        .bind(id())
        .bind(Grader_eval::Status::ready)
        .resultValue();
}


void Submission::load_cache() const
{
    if (!is_loaded_) reload_cache();
}

void Submission::clear_cache() const
{
    is_loaded_ = false;
    byte_count_ = -1;
}

void Submission::reload_cache() const
{
    item_count_  = 0;
    point_value_ = 0;
    items_.clear();

    for (const auto& eval_item : assignment()->eval_items()) {
        ++item_count_;
        point_value_ += eval_item->relative_value();

        auto sequence = eval_item->sequence();
        while (items_.size() <= sequence) items_.emplace_back();

        items_[sequence].submission = self();
        items_[sequence].eval_item = eval_item;
    }

    double total_grade = 0;

    Eval_counts counts {
        .item      = item_count_,
        .self      = 0,
        .grader    = 0,
    };

    for (const auto& self_eval : self_evals_) {
        auto const& eval_item = self_eval->eval_item();

        ++counts.self;

        auto sequence = eval_item->sequence();
        items_[sequence].self_eval = self_eval;

        auto grader_eval = self_eval->grader_eval();
        if (grader_eval) {
            items_[sequence].grader_eval = grader_eval;
            if (grader_eval->status() == Grader_eval::Status::ready) {
                ++counts.grader;
                total_grade +=
                    grader_eval->score() * eval_item->relative_value();
            }
        }
    }

    eval_status_    = eval_status_given_counts_(counts);
    is_ready_       = is_ready_given_counts_(counts);
    grade_          = clean_grade(total_grade, point_value_);
    is_loaded_      = true;
}

int Submission::assignment_number() const
{
    return assignment()->number();
}

std::string Submission::owner_string() const
{
    std::string result = user1()->name();
    if (user2()) {
        result += '+';
        result += user2()->name();
    }

    return result;
}

std::string Submission::owner_string(Viewing_context const& cxt) const
{
    if (cxt.viewer == user1() || cxt.viewer == user2())
        return "you";

    switch (cxt.viewer->role()) {
    case User::Role::Student:
        return "other student";
    case User::Role::Grader:
        return "student";
    case User::Role::Admin:
        return owner_string();
    }
}

std::string Submission::grade_string() const
{
    return percentage(grade());
}

namespace {

struct Partition_of_files
{
    vector<dbo::ptr<File_meta>> cheap;
    vector<string>              precious;

    Partition_of_files&
    operator<<(dbo::ptr<File_meta> const& file)
    {
        if (file->is_automatically_deletable())
            cheap.push_back(file);
        else
            precious.push_back(file->name());

        return *this;
    }
};

Partition_of_files partition_files(dbo::ptr<Submission> const& sub1,
                                   dbo::ptr<Submission> const& sub2)
{
    set<dbo::ptr<File_meta>, File_meta::Less_by_name<>>
            set1(begin(sub1->source_files()), end(sub1->source_files()));

    Partition_of_files partition;

    for (const auto& file2 : sub2->source_files()) {
        if (auto iter = set1.find(file2);
                iter != end(set1)) {

            auto& file1 = *iter;

            if (file1->is_automatically_deletable()
                && file2->is_automatically_deletable())
            {
                partition.cheap.push_back(file1);
                partition.cheap.push_back(file2);
            } else {
                partition.precious.push_back(file1->name());
            }
        }
    }

    return partition;
}

void ensure_disjoint(dbo::ptr<Submission> const& sub1,
                     dbo::ptr<Submission> const& sub2)
{
    auto partition = partition_files(sub1, sub2);

    if (partition.precious.empty()) {
        for (auto file : partition.cheap) {
            file.remove();
        }
    } else {
        throw Join_collision_error(sub1, sub2, partition.precious);
    }
}

} // end inline namespace

bool Submission::join_together(dbo::ptr<Submission> keep,
                               dbo::ptr<Submission> kill)
{
    if (keep->user2() || kill->user2()) return false;
    if (keep->user1() == kill->user1()) return false;
    if (keep->assignment() != kill->assignment()) return false;

    ensure_disjoint(keep, kill);

    for (auto file : kill->source_files()) {
        file.modify()->move(keep, file->name());
    }

    keep.modify()->user2_ = kill->user1_;
    kill.remove();

    return true;
}

dbo::ptr<Submission> Submission::find_this() const
{
    return session()->find<Submission>().where("id = ?").bind(id());
}

dbo::ptr<File_meta>
Submission::find_file_by_name(const std::string& filename) const
{
    return session()->find<File_meta>()
                    .where("name = ?").bind(filename)
                    .where("submission_id = ?").bind(id());
}

int Submission::remaining_space() const
{
    return bytes_quota() - byte_count();
}

bool Submission::has_sufficient_space(int bytes,
                                      const std::string& filename) const
{
    int remaining = remaining_space();

    auto existing_file = find_file_by_name(filename);
    if (existing_file) remaining += existing_file->byte_count();

    return remaining >= bytes;
}

std::string Submission::rest_uri() const
{
    return api::paths::Submissions_1(id());
}

std::string Submission::files_rest_uri() const
{
    return api::paths::Submissions_1_files(id());
}

std::string Submission::evals_rest_uri() const
{
    return api::paths::Submissions_1_evals(id());
}

J::Object Submission::to_json(bool brief) const
{
    J::Object result;

    result["id"]                  = J::Value(id());
    result["uri"]                 = J::Value(rest_uri());
    result["assignment_number"]   = J::Value(assignment()->number());
    result["status"]              = J::Value(stringify(status()));
    result["grade"]               = J::Value(grade());
    result["owner1"]              = J::Value(user1()->to_json(true));
    if (user2()) result["owner2"] = J::Value(user2()->to_json(true));

    if (!brief) {
        result["open_date"]     = J::Value(json_format(open_date()));
        result["due_date"]      = J::Value(json_format(effective_due_date()));
        result["eval_date"]     = J::Value(json_format(effective_eval_date()));
        result["last_modified"] = J::Value(json_format(last_modified()));
        result["files_uri"]     = J::Value(files_rest_uri());
        result["evals_uri"]     = J::Value(evals_rest_uri());
        result["bytes_used"]    = J::Value(byte_count());
        result["bytes_quota"]   = J::Value(bytes_quota());
        result["eval_status"]   = J::Value(stringify(eval_status()));
    }

    return result;
}

void
Submission::set_from_json(std::string_view field, Wt::Json::Value const& value)
{
    if (field == "due_date") {
        string time_spec = value;

        if (!set_due_date(time_spec).has_value())
            throw Cannot_set_field(
                    "due_date", "Could not parse timespec", value);
    }

    else if (field == "eval_date") {
        string time_spec = value;

        if (!set_eval_date(time_spec).has_value())
            throw Cannot_set_field(
                    "eval_date", "Could not parse timespec", value);
    }

    else if (field == "bytes_quota") {
        int quota = value;

        if (quota <= 0)
            throw Cannot_set_field(
              "bytes_quota", "Must be positive", value);

        set_bytes_quota(quota);
    }

    else if (field == "owner2") {
        if (!value.isNull())
            throw Cannot_set_field("owner2", "Must be null", value);

        try {
            divorce();
        } catch (No_partner_to_separate const&) {
            // ignore
        }
    }

    else {
        throw Cannot_set_field(string(field), "Unknown field");
    }
}

optional<WDateTime>
Submission::set_due_date(string_view date_string)
{
    if (WDateTime date; json_parse(date, date_string)) {
        set_due_date(date);
        return {date};
    } else {
        return nullopt;
    }
}

optional<WDateTime>
Submission::set_eval_date(string_view date_string)
{
    if (WDateTime date; json_parse(date, date_string)) {
        set_eval_date(date);
        return {date};
    } else {
        return nullopt;
    }
}

void Submission::clear_files()
{
    source_files_.clear();
    is_loaded_ = false;
}

void Submission::give_back_files(const Wt::Dbo::ptr<Submission>& dst_submission)
{
    auto const& src_user = user1();
    auto const& dst_user = dst_submission->user1();

    vector<dbo::ptr<File_meta>> whose_are_these;

    for (auto const& file : source_files_) {
        if (file->uploader() != src_user) {
            if (file->uploader() == dst_user) {
                source_files_.erase(file);
                dst_submission.modify()->source_files_.insert(file);
            } else if (file->uploader() != src_user) {
                if (file->purpose() != File_purpose::log) {
                    whose_are_these.push_back(file);
                    dbo::ptr(file).remove();
                }
                // TODO: this is dumb. move it inside the `if` above here
            }
        }
    }

    if (!whose_are_these.empty()) {
        throw Whose_files_are_these{whose_are_these};
    }

    is_loaded_ = false;
}

void Submission::divorce()
{
    auto old_user2 = user2();
    if (! old_user2)
        throw No_partner_to_separate{user1()->name()};

    set_user2({});
    auto other = session()->addNew<Submission>(old_user2, assignment());
    give_back_files(other);

    touch();
}

Self_eval_vec Submission::self_eval_vec() const
{
    Self_eval_vec result;

    auto insert = [&](dbo::ptr<Self_eval> const& ptr) {
        auto index = ptr->eval_item()->sequence();

        while (index >= result.size()) {
            result.emplace_back();
        }

        result[index] = ptr;
    };

    auto present = [&](size_t index) {
        return index < result.size() && result[index];
    };

    for (const auto& self_eval : self_evals_) {
        insert(self_eval);
    }

    for (const auto& eval_item : assignment()->eval_item_vec()) {
        auto index = eval_item->sequence();

        if (eval_item->type() == Eval_item::Type::Informational &&
            !present(index)) {
            insert(Submission::get_self_eval(eval_item, find_this()));
        }
    }

    return result;
}

char const* Enum<Submission::Status>::show(Submission::Status status)
{
    using S = Submission::Status;

    switch (status) {
        case S::future:
            return "future";
        case S::open:
            return "open";
        case S::extended:
            return "extended";
        case S::overtime:
            return "overtime";
        case S::self_eval:
            return "self_eval";
        case S::extended_eval:
            return "extended_eval";
        case S::closed:
            return "closed";
    }
}

char const* Enum<Submission::Eval_status>::show(Submission::Eval_status status)
{
    using S = Submission::Eval_status;

    switch (status) {
        case S::empty:
            return "empty";
        case S::overdue:
            return "overdue";
        case S::started:
            return "started";
        case S::complete:
            return "complete";
    }
}

bool Submission::Item::fully_frozen() const
{
    return self_eval && self_eval->fully_frozen();
}

bool Submission::Item::score_frozen() const
{
    return self_eval && self_eval->score_frozen();
}

optional<Submission::Item::View>
Submission::Item::view_self_eval(Viewing_context const& cxt,
                                 string const& not_set) const
{
    if (eval_item->is_informational()) {
        return nullopt;
    }

    View result;

    result.owner = submission->owner_string(cxt);

    if (self_eval) {
        if (cxt.viewer->role() == User::Role::Grader) {
            result.score = "[***]";
        } else {
            result.score = self_eval->plain_score_string();
            result.explanation = self_eval->explanation();
        }
    } else {
        result.score = not_set;
    }

    return result;
}

optional<Submission::Item::View>
Submission::Item::view_grader_eval(Viewing_context const& cxt,
                                   string const& not_set) const
{
    if (eval_item->is_graded_automatically() || !self_eval) {
        return nullopt;
    }

    View result;

    if (grader_eval &&
        (grader_eval->is_ready() ||
         cxt.viewer->can_grade()))
    {
        auto& grader = grader_eval->grader();
        if (! grader->can_grade()) {
            result.owner = "auto";
        } else if (cxt.viewer->can_grade() || grader->can_admin()) {
            result.owner = grader->nice_name(false);
        }

        result.score = grader_eval->plain_score_string();
        result.explanation = grader_eval->explanation();
    } else {
        result.owner = "grader";
        result.score = not_set;
    }

    return {result};
}


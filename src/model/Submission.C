#include "Submission.h"
#include "Self_eval.h"
#include "Assignment.h"
#include "Eval_item.h"
#include "File_data.h"
#include "File_meta.h"
#include "Grader_eval.h"
#include "auth/User.h"
#include "../Session.h"
#include "../common/format.h"
#include "../common/paths.h"

#include <Wt/WDateTime.h>
#include <Wt/Dbo/Impl.h>

#include <functional>
#include <locale>
#include <sstream>

namespace dbo = Wt::Dbo;
namespace J   = Wt::Json;

DBO_INSTANTIATE_TEMPLATES(Submission)

static int const initial_bytes_quota = 20 * 1024 * 1024;

Submission::Submission(const dbo::ptr <User>& user,
                       const dbo::ptr <Assignment>& assignment)
        : user1_(user), assignment_(assignment),
          due_date_(assignment->due_date()),
          eval_date_(assignment->eval_date()),
          last_modified_(Wt::WDateTime::currentDateTime()),
          bytes_quota_(initial_bytes_quota)
{

}

Source_file_vec Submission::source_files_sorted(bool name_only) const
{
    Source_file_vec result;

    for (const auto& ptr : source_files_)
        result.push_back(ptr);

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
            if (! meta->uploader()->can_admin())
                byte_count_ += meta->byte_count();
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
    return stat == Status::open || stat == Status::extended;
}

bool Submission::in_eval_period() const
{
    auto stat = status();
    return stat == Status::self_eval || stat == Status::extended_eval;
}

Submission::Status Submission::status() const
{
    auto now = Wt::WDateTime::currentDateTime();

    if (now <= assignment()->open_date())
        return Status::future;
    if (now <= assignment()->due_date())
        return Status::open;
    else if (now <= due_date_)
        return Status::extended;
    else if (now <= assignment()->eval_date())
        return Status::self_eval;
    else if (now <= eval_date_)
        return Status::extended_eval;
    else
        return Status::closed;
}

Submission::Eval_status Submission::eval_status() const
{
    if (is_loaded_) {
        return eval_status_;
    }

    size_t self_evals_size = self_evals_.size();

    if (self_evals_size == assignment()->eval_items().size())
        return Eval_status::complete;
    else if (self_evals_size > 0)
        return Eval_status::started;
    else
        return Eval_status::empty;
}

double Submission::grade() const
{
    if (is_loaded_) {
        return grade_;
    }

    return clean_grade(session()->query<double>(
            "SELECT SUM(relative_value * g.score) / NULLIF(SUM(relative_value), 0)"
            "  FROM self_eval s"
            " INNER JOIN eval_item e ON s.eval_item_id = e.id"
            " INNER JOIN grader_eval g ON g.self_eval_id = s.id"
            " WHERE s.submission_id = ?"
    ).bind(id()).resultValue());
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

Wt::Dbo::ptr<Self_eval>
Submission::get_self_eval(int sequence,
                          const dbo::ptr<Submission> &submission,
                          bool create)
{
    submission->load_cache();
    auto eval_item = submission->items_.at(sequence).eval_item;
    return get_self_eval(eval_item, submission, create);
}

void Submission::retract_self_eval(const dbo::ptr<Self_eval>& self_eval)
{
    auto submission = self_eval->submission();
    auto sequence = self_eval->eval_item()->sequence();

    submission->load_cache();

    submission->items_[sequence].grader_eval.remove();
    submission->items_[sequence].grader_eval = {};
    submission->items_[sequence].self_eval.remove();
    submission->items_[sequence].self_eval = {};

    submission.modify()->touch();
}

void
Submission::save_self_eval(const dbo::ptr<Self_eval>& self_eval,
                           Session& session,
                           double score,
                           const std::string& explanation)
{
    {
        auto self_eval_m = self_eval.modify();
        self_eval_m->set_score(score);
        self_eval_m->set_explanation(explanation);
        score = self_eval_m->score();
    }

    if (score == 0.0 &&
            self_eval->eval_item()->type() == Eval_item::Type::Boolean) {
        set_grader_eval(self_eval, session.user(), 0.1, "You chose no.");
    } else if (! self_eval->eval_item()->is_informational()) {
        retract_grader_eval(self_eval->grader_eval());
    }

    self_eval->submission().modify()->touch();
}

dbo::ptr<Grader_eval>&
Submission::find_grader_eval_(const dbo::ptr<Self_eval>& self_eval)
{
    auto submission = self_eval->submission();
    auto sequence = self_eval->eval_item()->sequence();

    submission->load_cache();

    return submission->items_[sequence].grader_eval;
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
Submission::set_grader_eval(const Wt::Dbo::ptr<Self_eval>& self_eval,
                            const Wt::Dbo::ptr<User>& grader,
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

void Submission::retract_grader_eval(const dbo::ptr<Grader_eval>& grader_eval)
{
    if (grader_eval->eval_item()->is_graded_automatically())
        return;

    auto submission = grader_eval->submission();
    auto sequence = grader_eval->eval_item()->sequence();

    submission->load_cache();

    submission->items_[sequence].grader_eval.remove();
    submission->items_[sequence].grader_eval = {};

    submission.modify()->touch();
}

void Submission::touch()
{
    last_modified_ = Wt::WDateTime::currentDateTime();
}

dbo::ptr<Submission>
Submission::find_by_assignment_and_user(dbo::Session& session,
                                        const dbo::ptr<Assignment>& assignment,
                                        const dbo::ptr<User>& user)
{
    dbo::ptr<Submission> result =
            session.find<Submission>()
                   .where("user1_id = ? OR user2_id = ?")
                   .bind(user.id()).bind(user.id())
                   .where("assignment_number = ?")
                   .bind(assignment->number());
    if (!result)
        result = session.addNew<Submission>(user, assignment);

    return result;
}

dbo::ptr<Submission> Submission::find_by_id(dbo::Session& session,
                                            int submission_id)
{
    return session.find<Submission>()
            .where("id = ?").bind(submission_id);
}

bool Submission::can_view(const dbo::ptr<User>& user) const
{
    return user->can_admin() || user == user1_ || user == user2_;
}

bool Submission::can_submit(const dbo::ptr<User>& user) const
{
    if (user->can_admin()) return true;
    if (user != user1_ && user != user2_) return false;

    return in_submit_period();
}

bool Submission::can_eval(const dbo::ptr <User>& user) const
{
    if (user->can_admin()) return true;
    if (user != user1_ && user != user2_) return false;

    return in_eval_period();
}

bool Submission::can_view_eval(const dbo::ptr<User>& user) const
{
    auto now = Wt::WDateTime::currentDateTime();

    return user->can_admin() ||
           (effective_due_date() < now &&
            (user == user1_ || user == user2_));
}

std::string Submission::url() const
{
    return url_for_user(user1_);
}

std::string Submission::eval_url() const
{
    return url() + "/eval";
}

std::string Submission::url_for_user(const dbo::ptr<User>& user) const
{
    std::ostringstream result;

    result << "/~" << (user == user2_? user2_ : user1_)->name();
    result << "/hw/" << assignment_->number();

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

bool Submission::is_graded() const
{
    return grading_status() == Grading_status::complete;
}

Submission::Grading_status Submission::grading_status() const
{
    if (is_loaded_) {
        return grading_status_;
    }

    auto grader_eval_count = session()->query<int>(
            "SELECT COUNT(*)"
            "  FROM self_eval s"
            " INNER JOIN grader_eval g ON g.self_eval_id = s.id"
            " WHERE s.submission_id = ?"
            "   AND g.status = ?"
    ).bind(id()).bind((int) Grader_eval::Status::ready).resultValue();

    if (grader_eval_count == self_evals_.size())
        return Grading_status::complete;

    if (grader_eval_count > 2 && in_eval_period()) {
        return Grading_status::regrade;
    }

    return Grading_status::incomplete;
}

void Submission::load_cache() const
{
    if (is_loaded_) return;

    item_count_ = 0;
    grade_ = 0;
    point_value_ = 0;

    for (const auto& eval_item : assignment()->eval_items()) {
        ++item_count_;
        point_value_ += eval_item->relative_value();
        auto sequence = eval_item->sequence();
        while (items_.size() <= sequence) items_.emplace_back();
        items_[sequence].eval_item = eval_item;
    }

    size_t self_eval_count = 0;
    size_t grader_eval_count = 0;

    for (const auto& self_eval : self_evals_) {
        ++self_eval_count;

        auto sequence = self_eval->eval_item()->sequence();
        items_[sequence].self_eval = self_eval;

        auto grader_eval = self_eval->grader_eval();
        if (grader_eval) {
            items_[sequence].grader_eval = grader_eval;
            if (grader_eval->status() == Grader_eval::Status::ready) {
                ++grader_eval_count;
                grade_ += grader_eval->score() *
                          self_eval->eval_item()->relative_value();
            }
        }
    }

    eval_status_ = self_eval_count == item_count_ ? Eval_status::complete
                 : self_eval_count == 0           ? Eval_status::empty
                 /* otherwise */                  : Eval_status::started;


    grading_status_ = grader_eval_count == item_count_ ? Grading_status::complete
                    : grader_eval_count > 2 && in_eval_period()
                                                       ? Grading_status::regrade
                    /* otherwise */                    : Grading_status::incomplete;

    grade_ = clean_grade(grade_ / point_value_);

    is_loaded_ = true;
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

std::string Submission::grade_string() const
{
    return percentage(grade());
}

bool Submission::join_together(dbo::ptr<Submission> keep,
                               dbo::ptr<Submission> kill)
{
    if (keep->user2() || kill->user2()) return false;
    if (keep->user1() == kill->user1()) return false;
    if (keep->assignment() != kill->assignment()) return false;

    for (dbo::ptr<File_meta> file : kill->source_files_sorted())
        file.remove();

    for (dbo::ptr<File_meta> file : keep->source_files_sorted())
        file.remove();

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

std::string Submission::rest_uri() const {
    return api::paths::Submissions_1(id());
}

std::string Submission::files_rest_uri() const {
    return api::paths::Submissions_1_files(id());
}

std::string Submission::evals_rest_uri() const {
    return api::paths::Submissions_1_evals(id());
}

J::Object Submission::to_json(bool brief) const
{
    J::Object result;

    result["id"] = J::Value(id());
    result["uri"] = J::Value(rest_uri());
    result["assignment_number"] = J::Value(assignment()->number());
    result["status"] = J::Value(stringify(status()));
    result["grade"] = J::Value(grade());
    result["owner1"] = J::Value(user1()->to_json(true));
    if (user2()) result["owner2"] = J::Value(user2()->to_json(true));

    if (!brief) {
        result["open_date"] = J::Value(json_format(open_date()));
        result["due_date"] = J::Value(json_format(effective_due_date()));
        result["eval_date"] = J::Value(json_format(effective_eval_date()));
        result["last_modified"] = J::Value(json_format(last_modified()));
        result["files_uri"] = J::Value(files_rest_uri());
        result["evals_uri"] = J::Value(evals_rest_uri());
        result["bytes_used"] = J::Value(byte_count());
        result["bytes_quota"] = J::Value(bytes_quota());
        result["eval_status"] = J::Value(stringify(eval_status()));
    }

    return result;
}


void Submission::clear_files()
{
    source_files_.clear();
    is_loaded_ = false;
}

bool Submission::divorce()
{
    if (user2()) {
        set_user2({});
        clear_files();
        touch();
        return true;
    } else {
        return false;
    }
}

Self_eval_vec Submission::self_eval_vec() const {
    Self_eval_vec result;

    auto insert = [&](dbo::ptr<Self_eval> const& ptr) {
        auto index = ptr->eval_item()->sequence();

        while (index >= result.size())
            result.emplace_back();

        result[index] = ptr;
    };

    auto present = [&](size_t index) {
        return index < result.size() && result[index];
    };

    for (const auto& self_eval : self_evals_)
        insert(self_eval);

    for (const auto& eval_item : assignment()->eval_item_vec()) {
        auto index = eval_item->sequence();

        if (eval_item->type() == Eval_item::Type::Informational && !present(index)) {
            insert(Submission::get_self_eval(eval_item, find_this()));
        }
    }

    return result;
}

char const* Enum<Submission::Status>::show(Submission::Status status)
{
    using S = Submission::Status;

    switch (status) {
        case S::future: return "future";
        case S::open: return "open";
        case S::self_eval: return "self_eval";
        case S::extended: return "extended";
        case S::extended_eval: return "extended_eval";
        case S::closed: return "closed";
    }
}

char const* Enum<Submission::Eval_status>::show(Submission::Eval_status status)
{
    using S = Submission::Eval_status;

    switch (status) {
        case S::empty: return "empty";
        case S::started: return "started";
        case S::complete: return "complete";
    }
}


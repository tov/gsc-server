#include "Submission.h"
#include "Self_eval.h"
#include "Assignment.h"
#include "Eval_item.h"
#include "File_data.h"
#include "File_meta.h"
#include "Grader_eval.h"
#include "auth/User.h"

#include <Wt/WDateTime>
#include <Wt/Dbo/Impl>

#include <algorithm>
#include <functional>
#include <locale>
#include <sstream>

DBO_INSTANTIATE_TEMPLATES(Submission);

Submission::Submission(const dbo::ptr <User>& user,
                       const dbo::ptr <Assignment>& assignment)
        : user1_(user), assignment_(assignment),
          due_date_(assignment->due_date()),
          eval_date_(assignment->eval_date()),
          last_modified_(Wt::WDateTime::currentDateTime())
{

}

Source_file_vec Submission::source_files_sorted() const
{
    Source_file_vec result;

    for (const auto& ptr : source_files_)
        result.push_back(ptr);

    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return std::lexicographical_compare(
                a->name().begin(), a->name().end(),
                b->name().begin(), b->name().end(),
                boost::is_iless()
        );
    });

    return result;
}

size_t Submission::file_count() const
{
    return source_files().size();
}

bool Submission::extended() const
{
    return due_date_ > assignment_->due_date();
}

bool Submission::eval_extended() const
{
    return eval_date_ > assignment_->eval_date();
}

const Wt::WDateTime& Submission::effective_due_date() const
{
    return std::max(due_date_, assignment_->due_date());
}

const Wt::WDateTime& Submission::effective_eval_date() const
{
    return std::max(eval_date_, assignment_->eval_date());
}

Submission::status Submission::get_status() const
{
    auto now = Wt::WDateTime::currentDateTime();

    if (now <= assignment()->open_date())
        return status::future;
    if (now <= assignment()->due_date())
        return status::open;
    else if (now <= due_date_)
        return status::extended;
    else if (now <= assignment()->eval_date())
        return status::self_eval;
    else if (now <= eval_date_)
        return status::extended_eval;
    else
        return status::closed;
}

Submission::eval_status Submission::get_eval_status() const
{
    size_t self_evals_size = self_evals().size();

    if (self_evals_size == assignment()->eval_items().size())
        return eval_status::complete;
    else if (self_evals_size > 0)
        return eval_status::started;
    else
        return eval_status::empty;
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
        result = session.add(new Submission(user, assignment));

    return result;
}

bool Submission::can_view(const dbo::ptr<User>& user) const
{
    return user->can_admin() || user == user1_ || user == user2_;
}

bool Submission::can_submit(const dbo::ptr<User>& user) const
{
    auto now = Wt::WDateTime::currentDateTime();

    return user->can_admin() ||
            (now <= effective_due_date() &&
                    (user == user1_ || user == user2_));
}

bool Submission::can_eval(const dbo::ptr<User>& user) const
{
    auto now = Wt::WDateTime::currentDateTime();

    return user->can_admin() ||
           (effective_due_date() < now &&
                   now <= effective_eval_date() &&
                   (user == user1_ || user == user2_));
}

std::string Submission::url(const dbo::ptr<User>& current) const
{
    std::ostringstream result;

    if (current != user1_ && current != user2_)
        result << "/~" << user1_->name();

    result << "/hw/" << assignment_->number();

    return result.str();
}

std::string Submission::eval_url(const dbo::ptr<User>& current) const
{
    return url(current) + "/eval";
}

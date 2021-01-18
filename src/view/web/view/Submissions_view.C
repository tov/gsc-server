#include "Submissions_view.h"
#include "../Confirmation_dialog.h"
#include "../widget/Date_time_edit.h"
#include "../widget/Partner_notification_widget.h"
#include "../../../model/auth/User.h"
#include "../../../model/Assignment.h"
#include "../../../model/Exam_grade.h"
#include "../../../model/Submission.h"
#include "../../../Session.h"
#include "../../../common/util.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/Transaction.h>
#include <Wt/WApplication.h>
#include <Wt/WDateTime.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>

using Row_view = Submissions_view::Row_view;
using Eval_status = Submission::Eval_status;

namespace {

const char* style_if_past(WDateTime const& date)
{
    return date < WDateTime::currentDateTime()? "past" : "";
}

[[maybe_unused]]
bool is_eod(WTime const& time)
{
    return time.hour() == 23 && time.minute() == 59;
}

WString friendly(Wt::WDateTime const& date_time)
{
    return date_time.toLocalTime().toString();

    //if (is_eod(local_time.time())) {
    //    return local_time.toString("dddd d MMM (Z)");
    //} else {
    //}
}

} // end anonymous namespace


// Class for printing human friendly durations to or from right now.
class Now
{
public:
    Now();

    WString time_to(WDateTime const&);
    WString time_from(WDateTime const&);

    static WString time_from_to(WDateTime const&, WDateTime const&);

private:
    WDateTime now_;
};

Now::Now() : now_{WDateTime::currentDateTime()}
{ }

WString Now::time_to(WDateTime const& to)
{
    return time_from_to(now_, to);
}

WString Now::time_from(WDateTime const& from)
{
    return time_from_to(from, now_);
}

WString Now::time_from_to(WDateTime const& from, WDateTime const& to)
{
    return from.timeTo(to, chrono::seconds{2});
}


Submissions_view::Model::Item::Item(
        dbo::ptr<Submission> const& submission,
        dbo::ptr<User> const& principal)
        : submission(submission)
        , file_count(submission->file_count())
        , eval_status(submission->eval_status())
        , principal(principal)
{ }

Submissions_view::Model::Model(
        dbo::ptr<User> const& principal,
        Session& session)
        : principal(principal)
{
    dbo::Transaction transaction(session);

    for (const auto& submission : principal->submissions()) {
        // Make sure these are loaded now.
        submission->user1()->id();
        if (submission->user2())submission->user2()->id();

        submissions[submission->assignment()->number()] = Item(submission, principal);
    }

    for (auto exam_grade : Exam_grade::find_by_user(principal)) {
        exams.push_back(exam_grade);
    }
}

Row_view::Row_view(
        const Row_model& model,
        Session& session,
        WTableRow* row)
        : model_(model),
          session_(session),
          row_(row)
{
    row->elementAt(NAME)->addNew<WText>(assignment()->name());
    status_ = row_->elementAt(STATUS)->addNew<WText>();
    grade_  = row_->elementAt(GRADE)->addNew<WText>();
    files_btn_ = row_->elementAt(FILES)->addNew<WPushButton>();
    eval_btn_ = row_->elementAt(EVAL)->addNew<WPushButton>();

    files_btn_->clicked().connect([this] { on_files_(); });
    eval_btn_->clicked().connect([this] { on_eval_(); });

    row->elementAt(GRADE)->setStyleClass("numeric");
}

void Row_view::add_headings(WTableRow* row)
{
    row->elementAt(STATUS)   ->addNew<WText>("Status");
    row->elementAt(DUE_DATE) ->addNew<WText>("Code Due");
    row->elementAt(EVAL_DATE)->addNew<WText>("Self-Eval Due");
    row->elementAt(GRADE)    ->addNew<WText>("Score");
    row->elementAt(FILES)    ->addNew<WText>("Files");
    row->elementAt(EVAL)     ->addNew<WText>("Eval");
}

void Row_view::set_files_button(const char* title, const char* style)
{
    files_btn_->setStyleClass(style);
    files_btn_->setText(title);
    files_btn_->show();
}

void Row_view::set_eval_button(const char* title, const char* style)
{
    eval_btn_->setStyleClass(style);
    eval_btn_->setText(title);
    eval_btn_->show();
}

void Row_view::update()
{
    update_styles_();
    update_status_();
    update_score_();
    update_buttons_();
}

void Row_view::update_styles_()
{
    row_->setStyleClass(get_row_style_());
    row_->elementAt(DUE_DATE)
        ->setStyleClass(style_if_past(submission()->effective_due_date()));
    row_->elementAt(EVAL_DATE)
        ->setStyleClass(style_if_past(submission()->effective_eval_date()));
}

void Row_view::update_status_()
{
    Now now;

    WString status;

    switch (submission()->status()) {
        case Status::future:
            status += "Opens in ";
            status += now.time_to(assignment()->open_date());
            break;

        case Status::open:
        case Status::extended:
            if (model_.file_count == 0) {
                status += "Due in ";
                status += now.time_to(submission()->effective_due_date());
            } else {
                status += "Submitted ";
                status += to_string(model_.file_count);
                status += model_.file_count > 1 ? " files" : " file";
            }
            break;

        case Status::self_eval:
        case Status::extended_eval:
            switch (model_.eval_status) {
                case Eval_status::empty: {
                    status += "Self-eval due in ";
                    status += now.time_to(submission()->effective_eval_date());
                    break;
                }

                case Eval_status::started: {
                    status += "Self-eval due in ";
                    status += now.time_to(submission()->effective_eval_date());
                    break;
                }

                case Eval_status::complete: {
                    status += "Self-eval complete";
                    break;
                }

                case Eval_status::overdue:
                    goto closed;
                    // Can't happen if the eval status is only overdue
                    // when the submission status is closed.
            }
            break;

        closed:
        case Status::closed: {
            status += "Closed ";
            status += now.time_from(submission()->effective_eval_date());
            status += " ago";
            break;
        }
    }

    status_->setText(status);
}

void Row_view::update_score_()
{
    dbo::Transaction transaction(session_);

    if (submission()->is_ready()) {
        grade_->setText(submission()->grade_string());
    }
}

static char const*
eval_action_from_status(Eval_status status)
{
    switch (status) {
    case Eval_status::empty:
        return "Start";
    case Eval_status::started:
        return "Continue";
    case Eval_status::complete:
        return "Edit";
    case Eval_status::overdue:
    default:
        return "View";
    }
}

static char const*
eval_button_style_from_status(Eval_status status)
{
    switch (status) {
    case Eval_status::empty:
    case Eval_status::started:
        return "btn btn-primary";
    case Eval_status::complete:
        return "btn btn-success";
    default:
        return "btn";
    }
}

void Row_view::update_buttons_()
{
    files_btn_->hide();
    eval_btn_->hide();

    char const* const eval_action =
        eval_action_from_status(model_.eval_status);

    switch (submission()->status()) {
        case Status::future:
            break;

        case Status::extended:
            set_eval_button(eval_action, "btn btn-danger");

            //
            // FALL THROUGH!
            //
        case Status::open:
            if (assignment()->web_allowed()) {
                set_files_button("Submit",
                        model_.file_count == 0
                        ? "btn btn-primary"
                        : "btn btn-info");
            } else {
                set_files_button("View", "btn");
            }
            break;

        case Status::self_eval:
        case Status::extended_eval: {
            char const* style = 
                eval_button_style_from_status(model_.eval_status);
            set_files_button("View", "btn");
            set_eval_button(eval_action, style);
            break;
        }

        case Status::closed:
            set_files_button("View", "btn");
            set_eval_button("View", "btn");
            break;
    }
}

WString Row_view::get_row_style_() const
{
    switch (submission()->status()) {
        case Status::future:
            return "future";

        case Status::self_eval:
        case Status::extended_eval:
            switch (model_.eval_status) {
                case Eval_status::empty:
                    return "self-eval needed";

                case Eval_status::started:
                    return "self-eval started";

                case Eval_status::complete:
                    return "";

                case Eval_status::overdue:
                    return "";
            }

        case Status::open:
        case Status::extended:
            return "open";

        case Status::closed:
            return "";
    }
}

class Student_submissions_view_row : public Row_view
{
public:
    Student_submissions_view_row(
            const Row_model& model,
            Session& session,
            WTableRow* row);
};

Student_submissions_view_row::Student_submissions_view_row(
        const Row_model& model, Session& session,
        WTableRow* row)
        : Submissions_view::Row_view(model, session, row)
{
    row_->elementAt(DUE_DATE)->addNew<WText>(
            friendly(submission()->effective_due_date()));
    row_->elementAt(EVAL_DATE)->addNew<WText>(
            friendly(submission()->effective_eval_date()));
}

class Admin_submissions_view_row : public Row_view
{
public:
    Admin_submissions_view_row(
            const Row_model& model,
            Session& session,
            WTableRow* row);

    void update() override;

protected:
    void set_files_button(const char*, const char*) override;
    void set_eval_button(const char*, const char*) override;

    Date_time_edit* due_date_;
    Date_time_edit* eval_date_;

    void due_date_changed_();
    void eval_date_changed_();
};

Admin_submissions_view_row::Admin_submissions_view_row(
        const Row_model& model,
        Session& session,
        WTableRow* row)
        : Submissions_view::Row_view(model, session, row)
{
    due_date_ = row->elementAt(DUE_DATE)->addNew<Date_time_edit>();
    eval_date_ = row->elementAt(EVAL_DATE)->addNew<Date_time_edit>();

    due_date_->set_date_format("MM/dd/yy");
    eval_date_->set_date_format("MM/dd/yy");

    due_date_->set_time_format("HH:mm");
    eval_date_->set_time_format("HH:mm");

    due_date_->changed().connect(this,
                                 &Admin_submissions_view_row::due_date_changed_);
    eval_date_->changed().connect(this,
                                  &Admin_submissions_view_row::eval_date_changed_);
}

void Admin_submissions_view_row::update()
{
    Row_view::update();
    due_date_->set_date_time(submission()->effective_due_date());
    eval_date_->set_date_time(submission()->effective_eval_date());

    if (submission()->extended()) {
        due_date_->setStyleClass("extended");
        auto date = friendly(assignment()->due_date());
        due_date_->setToolTip("Extended from " + date);
    } else {
        due_date_->setStyleClass("");
        due_date_->setToolTip("");
    }

    if (submission()->eval_extended()) {
        eval_date_->setStyleClass("extended");
        auto date = friendly(assignment()->eval_date());
        eval_date_->setToolTip("Extended from " + date);
    } else {
        eval_date_->setStyleClass("");
        eval_date_->setToolTip("");
    }
}

void Admin_submissions_view_row::due_date_changed_()
{
    if (due_date_->validate() == ValidationState::Valid) {
        due_date_->setStyleClass("");
        dbo::Transaction transaction(session_);
        submission().modify()->set_due_date(due_date_->date_time());
        transaction.commit();
        update();
    } else {
        due_date_->setStyleClass("invalid");
    }
}

void Admin_submissions_view_row::eval_date_changed_()
{
    if (eval_date_->validate() == ValidationState::Valid) {
        eval_date_->setStyleClass("");
        dbo::Transaction transaction(session_);
        submission().modify()->set_eval_date(eval_date_->date_time());
        transaction.commit();
        update();
    } else {
        eval_date_->setStyleClass("invalid");
    }
}

void Admin_submissions_view_row::set_files_button(const char*, const char*)
{
    Row_view::set_files_button("Go", "btn");
}

void Admin_submissions_view_row::set_eval_button(const char*, const char*)
{
    Row_view::set_eval_button("Go", "btn");
}

unique_ptr<Row_view>
Row_view::construct(const Row_model& model,
                    Session& session,
                    WTableRow* row)
{
    if (session.user()->can_admin()) {
        auto result = make_unique<Admin_submissions_view_row>(
                model, session, row);
        result->update();
        return move(result);
    } else {
        auto result = make_unique<Student_submissions_view_row>(
                model, session, row);
        result->update();
        return move(result);
    }
}

void Row_view::on_files_() const
{
    go_files_();
}

void Row_view::on_eval_() const
{
    if (submission()->status() == Status::extended)
        confirm_eval_();
    else
        go_eval_();
}

void Row_view::confirm_eval_() const
{

    std::ostringstream oss;
    oss << "Are you sure you want to end your extension for "
        << assignment()->name()
        << " submission?"
        << " Please note that this action is irreversible.";

    Confirmation_dialog::create(oss.str())
        .accepted().connect([this] { force_eval_now_(); });
}

void Row_view::force_eval_now_() const
{
    dbo::Transaction trans(session_);
    submission().modify()->end_extension_now();
    go_eval_();
}

void Row_view::go_files_() const
{
    Navigate::to(submission()->files_url_for_user(model_.principal));
}

void Row_view::go_eval_() const
{
    Navigate::to(submission()->eval_url_for_user(model_.principal));
}

Submissions_view::Submissions_view(const dbo::ptr<User>& user, Session& session)
        : session_(session)
        , model_{user, session}
{
    setStyleClass("submissions-view");
    changed_.connect(this, &Submissions_view::on_change_);
    reload_();
}

void Submissions_view::reload_()
{
    clear();

    addNew<Partner_notification_widget>(
            model_.principal, dbo::ptr<Submission>{}, session_, changed_);

    auto table = addNew<WTable>();
    table->setHeaderCount(1, Orientation::Horizontal);
    table->setHeaderCount(1, Orientation::Vertical);
    Row_view::add_headings(table->rowAt(0));

    int row = 1;
    for (const auto& each : model_.submissions) {
        if (!each.submission) continue;
        rows_.push_back(Row_view::construct(
                each, session_, table->rowAt(row++)));
    }

    auto exam_table = addNew<WTable>();
    exam_table->setStyleClass("exam-table");
    row = 0;
    for (const auto& each : model_.exams) {
        auto exam_row = exam_table->rowAt(row++);

        ostringstream fmt;
        fmt << "Exam " << each->number();
        exam_row->elementAt(0)->addNew<WText>(fmt.str());

        fmt.str("");
        fmt << each->points() << " / " << each->possible();
        exam_row->elementAt(1)->addNew<WText>(fmt.str());

        exam_row->elementAt(2)->addNew<WText>(each->pct_string());
    }
}

void Submissions_view::on_change_(Submission_change_message submission)
{
    if (submission) {
        dbo::Transaction trans(session_);
        model_.submissions[submission->assignment_number()] =
                Model::Item(submission, model_.principal);
    }

    reload_();
}

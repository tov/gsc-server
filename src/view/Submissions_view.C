#include "Submissions_view.h"
#include "Date_time_edit.h"
#include "../model/auth/User.h"
#include "../model/Assignment.h"
#include "../model/Submission.h"
#include "../model/Session.h"

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Transaction>
#include <Wt/WApplication>
#include <Wt/WDateTime>
#include <Wt/WLocalDateTime>
#include <Wt/WPushButton>
#include <Wt/WTable>
#include <Wt/WText>

#include <vector>

struct Submissions_view_model_item {
    Wt::Dbo::ptr<Submission> submission;
    size_t file_count = 0;
    Submission::eval_status eval_status;
};

using Submissions_view_model = std::vector<Submissions_view_model_item>;

void load_model(const Wt::Dbo::ptr<User>& user, Session& session,
                Submissions_view_model& result)
{
    Wt::Dbo::Transaction transaction(session);

    for (const auto& submission : user->submissions()) {
        int index = submission->assignment()->number();
        while (index >= result.size()) result.emplace_back();

        // Make sure this is loaded now.
        submission->user1()->name();

        result[index].submission  = submission;
        result[index].file_count  = submission->file_count();
        result[index].eval_status = submission->get_eval_status();
    }

    for (const auto& assignment : session.find<Assignment>().resultList()) {
        int index = assignment->number();
        while (index >= result.size()) result.emplace_back();

        if (! result[index].submission) {
            auto submission = new Submission(user, assignment);
            result[index].submission = session.add(submission);
        }
    }

    transaction.commit();
}

class Submissions_view_row : public Wt::WObject
{
public:
    enum columns { NAME, STATUS, DUE_DATE, EVAL_DATE, GRADE, ACTION, };

    static std::unique_ptr<Submissions_view_row>
    construct(const Submissions_view_model_item& model,
              Session& session,
              Wt::WTableRow* row);

    static void add_headings(Wt::WTableRow*);

protected:
    Submissions_view_row(const Submissions_view_model_item& model,
                         Session& session,
                         Wt::WTableRow* row);

    const Submissions_view_model_item& model_;
    Session& session_;
    Wt::WTableRow* row_;

    virtual void update();

    virtual void set_files_action(const char[]);
    virtual void set_eval_action(const char[]);
    virtual void set_action_style_class(const char[]);

private:
    Wt::WText* status_;
    Wt::WPushButton* action_;
    std::string action_url_;

    void action();
};

Submissions_view_row::Submissions_view_row(
        const Submissions_view_model_item& model,
        Session& session,
        Wt::WTableRow* row)
        : model_(model),
          session_(session),
          row_(row)
{
    new Wt::WText(model_.submission->assignment()->name(),
                  row->elementAt(NAME));
    status_ = new Wt::WText(row_->elementAt(STATUS));
    action_ = new Wt::WPushButton(row_->elementAt(ACTION));

    action_->clicked().connect(this, &Submissions_view_row::action);
}

void Submissions_view_row::add_headings(Wt::WTableRow* row)
{
    new Wt::WText("Assignment",     row->elementAt(NAME));
    new Wt::WText("Status",         row->elementAt(STATUS));
    new Wt::WText("Due",            row->elementAt(DUE_DATE));
    new Wt::WText("Self-eval due",  row->elementAt(EVAL_DATE));
    new Wt::WText("Grade",          row->elementAt(GRADE));
    new Wt::WText("Action",         row->elementAt(ACTION));
}

void Submissions_view_row::set_files_action(const char* title)
{
    action_->setText(title);
    action_url_ = model_.submission->url(session_.user());
}

void Submissions_view_row::set_eval_action(const char* title)
{
    action_->setText(title);
    action_url_ = model_.submission->eval_url(session_.user());
}

void Submissions_view_row::set_action_style_class(const char* style)
{
    action_->setStyleClass(style);
}

void Submissions_view_row::update()
{
    auto const now = Wt::WDateTime::currentDateTime();

    auto time_to = [&](const Wt::WDateTime& date) {
        return now.timeTo(date, 2);
    };

    action_->show();

    Wt::WString status;
    switch (model_.submission->get_status()) {
        case Submission::status::future:
            row_->setStyleClass("future");
            status += "Opens in ";
            status += time_to(model_.submission->assignment()->open_date());
            action_->hide();
            break;

        case Submission::status::open:
        case Submission::status::extended:
            set_files_action("Submit");
            if (model_.file_count == 0) {
                row_->setStyleClass("open");
                status += "Due in ";
                status += time_to(model_.submission->effective_due_date());
                set_action_style_class("btn-success");
            } else {
                row_->setStyleClass("open");
                status += "Submitted ";
                status += boost::lexical_cast<std::string>(model_.file_count);
                status += " file";
                if (model_.file_count > 1) status += "s";
                set_action_style_class("btn");
            }
            break;

        case Submission::status::self_eval:
        case Submission::status::extended_eval:
            switch (model_.eval_status) {
                case Submission::eval_status::empty: {
                    row_->setStyleClass("self-eval needed");
                    status += "Self-eval due in ";
                    status += time_to(model_.submission->effective_eval_date());
                    set_eval_action("Start");
                    set_action_style_class("btn-success");
                    break;
                }

                case Submission::eval_status::started: {
                    row_->setStyleClass("self-eval started");
                    status += "Self-eval due in ";
                    status += time_to(model_.submission->effective_eval_date());
                    set_eval_action("Continue");
                    set_action_style_class("btn-success");
                    break;
                }

                case Submission::eval_status::complete: {
                    row_->setStyleClass("self-eval complete");
                    status += "Self-eval complete";
                    set_eval_action("Edit");
                    set_action_style_class("btn");
                    break;
                }
            }
            break;

        case Submission::status::closed: {
            row_->setStyleClass("closed");
            status += "Closed ";
            status += model_.submission->effective_eval_date().timeTo(now, 2);
            status += " ago";
            set_eval_action("View");
            set_action_style_class("btn-link");
            break;
        }
    }

    status_->setText(status);

    row_->elementAt(DUE_DATE)->setStyleClass(
            model_.submission->effective_due_date() < now ? "past" : "");
    row_->elementAt(EVAL_DATE)->setStyleClass(
            model_.submission->effective_eval_date() < now ? "past" : "");
}

void Submissions_view_row::action()
{
    if (!action_url_.empty())
        Wt::WApplication::instance()->setInternalPath(action_url_, true);
}

class Student_submissions_view_row : public Submissions_view_row
{
public:
    Student_submissions_view_row(
            const Submissions_view_model_item& model,
            Session& session,
            Wt::WTableRow* row);
};

Student_submissions_view_row::Student_submissions_view_row(
        const Submissions_view_model_item& model, Session& session,
        Wt::WTableRow* row)
        : Submissions_view_row(model, session, row)
{
    new Wt::WText(model_.submission->effective_due_date().toLocalTime()
                        .toString("ddd, MMM d 'at' h:mm AP"),
                  row_->elementAt(DUE_DATE));
    new Wt::WText(model_.submission->effective_eval_date().toLocalTime()
                        .toString("ddd, MMM d 'at' h:mm AP"),
                  row_->elementAt(EVAL_DATE));
}

class Admin_submissions_view_row : public Submissions_view_row
{
public:
    Admin_submissions_view_row(
            const Submissions_view_model_item& model,
            Session& session,
            Wt::WTableRow* row);

    virtual void update() override;

protected:
    virtual void set_files_action(const char*) override;
    virtual void set_eval_action(const char*) override;
    virtual void set_action_style_class(const char*) override;

    Date_time_edit* due_date_;
    Date_time_edit* eval_date_;

    void due_date_changed_();
    void eval_date_changed_();
};

Admin_submissions_view_row::Admin_submissions_view_row(
        const Submissions_view_model_item& model,
        Session& session,
        Wt::WTableRow* row)
        : Submissions_view_row(model, session, row)
{
    due_date_ = new Date_time_edit(row->elementAt(DUE_DATE));
    eval_date_ = new Date_time_edit(row->elementAt(EVAL_DATE));

    due_date_->set_date_format("M/d/yy");
    eval_date_->set_date_format("M/d/yy");

    due_date_->set_time_format("H:mm");
    eval_date_->set_time_format("H:mm");

    due_date_->changed().connect(this,
                                 &Admin_submissions_view_row::due_date_changed_);
    eval_date_->changed().connect(this,
                                  &Admin_submissions_view_row::eval_date_changed_);
}

void Admin_submissions_view_row::update()
{
    Submissions_view_row::update();
    due_date_->set_date_time(model_.submission->effective_due_date());
    eval_date_->set_date_time(model_.submission->effective_eval_date());

    due_date_->set_top(eval_date_->date_time());
    eval_date_->set_bottom(due_date_->date_time());

    if (model_.submission->extended()) {
        due_date_->setStyleClass("extended");
        auto date = model_.submission->assignment()->due_date().toLocalTime().toString();
        due_date_->setToolTip("Extended from " + date);
    } else {
        due_date_->setStyleClass("");
        due_date_->setToolTip("");
    }

    if (model_.submission->eval_extended()) {
        eval_date_->setStyleClass("extended");
        auto date = model_.submission->assignment()->eval_date().toLocalTime().toString();
        eval_date_->setToolTip("Extended from " + date);
    } else {
        eval_date_->setStyleClass("");
        eval_date_->setToolTip("");
    }
}

void Admin_submissions_view_row::due_date_changed_()
{
    if (due_date_->validate() == Wt::WValidator::Valid) {
        due_date_->setStyleClass("");
        Wt::Dbo::Transaction transaction(session_);
        model_.submission.modify()->set_due_date(due_date_->date_time());
        transaction.commit();
        update();
    } else {
        due_date_->setStyleClass("invalid");
    }
}

void Admin_submissions_view_row::eval_date_changed_()
{
    if (eval_date_->validate() == Wt::WValidator::Valid) {
        eval_date_->setStyleClass("");
        Wt::Dbo::Transaction transaction(session_);
        model_.submission.modify()->set_eval_date(eval_date_->date_time());
        transaction.commit();
        update();
    } else {
        eval_date_->setStyleClass("invalid");
    }
}

void Admin_submissions_view_row::set_files_action(const char*)
{
    Submissions_view_row::set_files_action("Files");
}

void Admin_submissions_view_row::set_eval_action(const char*)
{
    Submissions_view_row::set_eval_action("Eval");
}

void Admin_submissions_view_row::set_action_style_class(const char*)
{
    Submissions_view_row::set_action_style_class("btn");
}

std::unique_ptr<Submissions_view_row>
Submissions_view_row::construct(const Submissions_view_model_item& model,
                                Session& session, Wt::WTableRow* row)
{
    if (session.user()->can_admin()) {
        auto result = std::make_unique<Admin_submissions_view_row>(
                model, session, row);
        result->update();
        return std::move(result);
    } else {
        auto result = std::make_unique<Student_submissions_view_row>(
                model, session, row);
        result->update();
        return std::move(result);
    }
}

Submissions_view::Submissions_view(const Wt::Dbo::ptr<User>& user,
                                   Session& session,
                                   Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          session_(session)
{
    setStyleClass("submissions-view");

    load_model(user, session, model_);

    auto table = new Wt::WTable(this);
    table->setHeaderCount(1);
    Submissions_view_row::add_headings(table->rowAt(0));

    int row = 1;
    for (const auto& each : model_) {
        if (!each.submission) continue;

        rows_.push_back(Submissions_view_row::construct(each, session,
                                                        table->rowAt(row++)));
    }
}

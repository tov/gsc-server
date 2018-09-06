#include "Assignments_view.h"
#include "Confirmation_dialog.h"
#include "Date_time_edit.h"
#include "../model/Assignment.h"
#include "../Navigate.h"

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WCheckBox.h>
#include <Wt/WDateTime.h>
#include <Wt/WLocalDateTime.h>
#include <Wt/WDialog.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>
#include <Wt/WTime.h>

#include <sstream>

class Assignments_view_row
{
public:
    Assignments_view_row(const dbo::ptr<Assignment>&,
                         Session&,
                         Wt::WTableRow*);

    enum columns
    {
        NUMBER,
        NAME,
        PARTNER,
        OPEN_DATE,
        DUE_DATE,
        EVAL_DATE,
        POINTS,
        ACTION,
    };

    static void add_headings(Wt::WTableRow*);

private:
    dbo::ptr<Assignment> assignment_;
    Session& session_;
    Wt::WTableRow* row_;
    Wt::WCheckBox* partner_;
    Wt::WLineEdit* name_;
    Date_time_edit* open_date_;
    Date_time_edit* due_date_;
    Date_time_edit* eval_date_;
    Wt::WLineEdit* points_;

    void update_() const;

    friend class Assignments_view;
};

Assignments_view_row::Assignments_view_row(
        const dbo::ptr<Assignment>& assignment,
        Session& session,
        Wt::WTableRow* row)
        : assignment_(assignment),
          session_(session),
          row_(row)
{
    new Wt::WText(boost::lexical_cast<std::string>(assignment_->number()),
                  row_->elementAt(NUMBER));
    name_ = new Wt::WLineEdit(row_->elementAt(NAME));
    partner_ = new Wt::WCheckBox(row_->elementAt(PARTNER));
    open_date_ = new Date_time_edit(row_->elementAt(OPEN_DATE));
    due_date_ = new Date_time_edit(row_->elementAt(DUE_DATE));
    eval_date_ = new Date_time_edit(row_->elementAt(EVAL_DATE));
    points_ = new Wt::WLineEdit(row_->elementAt(POINTS));
    auto edit = new Wt::WPushButton("Edit", row_->elementAt(ACTION));

    name_->setStyleClass("name");
    points_->setStyleClass("points");

    name_->changed().connect(std::bind([=] () {
        dbo::Transaction transaction(session_);
        assignment_.modify()->set_name(name_->text().toUTF8());
        transaction.commit();
        update_();
    }));

    partner_->changed().connect(std::bind([=] () {
        dbo::Transaction transaction(session_);
        assignment_.modify()->set_partner(partner_->isChecked());
        transaction.commit();
        update_();
    }));

    points_->changed().connect(std::bind([=] () {
        std::string points_s = points_->text().toUTF8();
        int points = atoi(points_s.data());
        dbo::Transaction transaction(session_);
        assignment_.modify()->set_points(points);
        transaction.commit();
        update_();
    }));

    open_date_->set_date_format("M/d/yy");
    due_date_->set_date_format("M/d/yy");
    eval_date_->set_date_format("M/d/yy");

    open_date_->set_time_format("H:mm");
    due_date_->set_time_format("H:mm");
    eval_date_->set_time_format("H:mm");

    open_date_->changed().connect(std::bind([=] () {
        if (open_date_->validate() == Wt::WValidator::Valid) {
            dbo::Transaction transaction(session_);
            assignment_.modify()->set_open_date(open_date_->date_time());
            transaction.commit();
            open_date_->setStyleClass("");
            update_();
        } else {
            open_date_->setStyleClass("invalid");
        }
    }));

    due_date_->changed().connect(std::bind([=] () {
        if (due_date_->validate() == Wt::WValidator::Valid) {
            dbo::Transaction transaction(session_);
            assignment_.modify()->set_due_date(due_date_->date_time());
            transaction.commit();
            due_date_->setStyleClass("");
            update_();
        } else {
            due_date_->setStyleClass("invalid");
        }
    }));

    eval_date_->changed().connect(std::bind([=] () {
        if (eval_date_->validate() == Wt::WValidator::Valid) {
            dbo::Transaction transaction(session_);
            assignment_.modify()->set_eval_date(eval_date_->date_time());
            transaction.commit();
            eval_date_->setStyleClass("");
            update_();
        } else {
            due_date_->setStyleClass("invalid");
        }
    }));

    std::ostringstream url;
    url << "/hw/" << assignment->number();
    edit->clicked().connect(Navigate(url.str()));

    update_();
}

void Assignments_view_row::add_headings(Wt::WTableRow* row)
{
    new Wt::WText("#",         row->elementAt(NUMBER));
    new Wt::WText("Name",      row->elementAt(NAME));
    (new Wt::WText("2",         row->elementAt(PARTNER)))
            ->setToolTip("Allow partners");
    new Wt::WText("Opens",     row->elementAt(OPEN_DATE));
    new Wt::WText("Due",       row->elementAt(DUE_DATE));
    new Wt::WText("Self-eval", row->elementAt(EVAL_DATE));
    new Wt::WText("Pts",       row->elementAt(POINTS));
    new Wt::WText("Action",    row->elementAt(ACTION));
}

void Assignments_view_row::update_() const
{
    name_->setText(assignment_->name());
    partner_->setChecked(assignment_->partner());
    open_date_->set_date_time(assignment_->open_date());
    due_date_->set_date_time(assignment_->due_date());
    eval_date_->set_date_time(assignment_->eval_date());
    points_->setText(boost::lexical_cast<std::string>(assignment_->points()));

//    open_date_->set_top(due_date_->date_time());
//    due_date_->set_bottom(open_date_->date_time());
//    due_date_->set_top(eval_date_->date_time());
//    eval_date_->set_bottom(due_date_->date_time());
}

Assignments_view::Assignments_view(Session& session,
                                   Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          session_(session)
{
    setStyleClass("assignments-view");

    dbo::Transaction transaction(session_);
    auto assignments = session_.find<Assignment>()
                               .orderBy("number")
                               .resultList();

    table_ = new Wt::WTable(this);
    table_->setHeaderCount(1);
    Assignments_view_row::add_headings(table_->rowAt(0));

    int row = 1;
    for (const auto& assignment : assignments) {
        rows_.push_back(std::make_unique<Assignments_view_row>(
                assignment, session_, table_->rowAt(row++)));
    }
    transaction.commit();

    auto buttons = new Wt::WContainerWidget(this);
    buttons->setStyleClass("buttons");

    auto more = new Wt::WPushButton("More", buttons);
    auto fewer = new Wt::WPushButton("Fewer", buttons);

    more->clicked().connect(this, &Assignments_view::more_);
    fewer->clicked().connect(this, &Assignments_view::fewer_);
}

void Assignments_view::more_()
{
    int number = (int)rows_.size() + 1;
    std::string name = "Homework " + boost::lexical_cast<std::string>(number);
    auto local_date = Wt::WLocalDateTime::currentDateTime();
    local_date.setTime(Wt::WTime(23, 59));
    auto date = local_date.toUTC();

    dbo::Transaction transaction(session_);
    auto assignment = session_.add(
            new Assignment(number, name, 0, date, date, date));
    transaction.commit();

    rows_.push_back(std::make_unique<Assignments_view_row>(
            assignment, session_, table_->rowAt(table_->rowCount())));
}

void Assignments_view::fewer_()
{
    if (rows_.empty()) return;

    auto assignment = rows_.back()->assignment_;

    dbo::Transaction transaction(session_);
    auto submission_count = assignment->submissions().size();
    auto eval_item_count = assignment->eval_items().size();
    transaction.commit();

    if (submission_count == 0 && eval_item_count == 0) {
        real_fewer_();
    } else {
        std::ostringstream message;
        message << assignment->name() << " currently has "
                << submission_count << " submission(s) and "
                << eval_item_count << " eval item(s). Are you sure you want "
                "to delete it?";

        auto dialog = new Confirmation_dialog(message.str(), this);
        dialog->accepted().connect(this, &Assignments_view::real_fewer_);
    }
}

void Assignments_view::real_fewer_()
{
    if (rows_.empty()) return;

    dbo::Transaction transaction(session_);
    rows_.back()->assignment_.remove();
    transaction.commit();

    table_->deleteRow(table_->rowCount() - 1);
    rows_.pop_back();
}

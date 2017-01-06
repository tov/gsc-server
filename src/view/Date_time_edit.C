#include "Date_time_edit.h"

#include <Wt/WLocalDateTime>

Date_time_edit::Date_time_edit(Wt::WContainerWidget* parent)
        : WCompositeWidget(parent)
{
    auto impl = new Wt::WContainerWidget;
    setImplementation(impl);

    date_edit_ = new Wt::WDateEdit(impl);
    time_edit_ = new Wt::WTimeEdit(impl);

    date_edit_->setStyleClass("date-edit");
    time_edit_->setStyleClass("time-edit");

    date_edit_->changed().connect(this, &Date_time_edit::fire_changed_);
    time_edit_->changed().connect(this, &Date_time_edit::fire_changed_);

    date_edit_->selected().connect(this, &Date_time_edit::fire_selected_);
    time_edit_->selected().connect(this, &Date_time_edit::fire_selected_);
}

void Date_time_edit::set_date_time(const Wt::WDateTime& date)
{
    auto local = date.toLocalTime();
    date_edit_->setDate(local.date());
    time_edit_->setTime(local.time());
}

Wt::WDateTime Date_time_edit::date_time() const
{
    return Wt::WLocalDateTime(date_edit_->date(), time_edit_->time()).toUTC();
}

Wt::WCalendar* Date_time_edit::calendar() const
{
    return date_edit_->calendar();
}

Wt::WValidator::State Date_time_edit::validate()
{
    if (date_edit_->text() == "" && time_edit_->text() == "") {
        if (mandatory_)
            return Wt::WValidator::InvalidEmpty;
        else
            return Wt::WValidator::Valid;
    }

    if (date_edit_->validate() != Wt::WValidator::Valid)
        return Wt::WValidator::Invalid;

    if (time_edit_->validate() != Wt::WValidator::Valid)
        return Wt::WValidator::Invalid;

    auto value = date_time();

    if (bottom_.isValid() && value < bottom_)
        return Wt::WValidator::Invalid;
    if (top_.isValid() && value > top_)
        return Wt::WValidator::Invalid;

    return Wt::WValidator::Valid;
}

void Date_time_edit::fire_changed_()
{
    changed_.emit();
}

void Date_time_edit::fire_selected_()
{
    selected_.emit();
}

void
Date_time_edit::setToolTip(const Wt::WString& text, Wt::TextFormat textFormat)
{
    date_edit_->setToolTip(text, textFormat);
    time_edit_->setToolTip(text, textFormat);
}

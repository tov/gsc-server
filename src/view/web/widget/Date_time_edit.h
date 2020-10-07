#pragma once

#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDateEdit.h>
#include <Wt/WTimeEdit.h>

class Date_time_validator;

class Date_time_edit : public Wt::WCompositeWidget {
public:
  Date_time_edit();

  void set_date_time(const Wt::WDateTime &);
  Wt::WDateTime date_time() const;

  virtual void setToolTip(const Wt::WString &,
                          Wt::TextFormat = Wt::TextFormat::Plain) override;

  Wt::WValidator::State validate();

  Wt::Signal<> &changed() { return changed_; }
  Wt::Signal<> &selected() { return selected_; }

  void set_date_format(const Wt::WString &fmt) { date_edit_->setFormat(fmt); }
  void set_time_format(const Wt::WString &fmt) { time_edit_->setFormat(fmt); }
  const Wt::WString date_format() const { return date_edit_->format(); }
  const Wt::WString time_format() const { return time_edit_->format(); }

  void set_bottom(const Wt::WDateTime &date) { bottom_ = date; }
  void set_top(const Wt::WDateTime &date) { top_ = date; }
  const Wt::WDateTime &bottom() const { return bottom_; }
  const Wt::WDateTime &top() const { return top_; }

  bool mandatory() const { return mandatory_; }
  void set_mandatory(bool b) { mandatory_ = b; }

  Wt::WCalendar *calendar() const;

private:
  Wt::WDateTime bottom_;
  Wt::WDateTime top_;
  Wt::WDateEdit *date_edit_;
  Wt::WTimeEdit *time_edit_;
  bool mandatory_ = true;

  Wt::Signal<> changed_;
  Wt::Signal<> selected_;

  void fire_changed_();
  void fire_selected_();
};

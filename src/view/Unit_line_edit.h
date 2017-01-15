#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WSignal>

class Unit_line_edit : public Wt::WCompositeWidget
{
public:
    Unit_line_edit(Wt::WContainerWidget* parent = nullptr);

    static constexpr double INVALID = -1.0;

    double value() const;
    void set_value(double);

    Wt::Signal<>& valid() { return valid_; }
    Wt::Signal<>& invalid() { return invalid_; }

private:
    Wt::WLineEdit* edit_;

    Wt::Signal<> valid_;
    Wt::Signal<> invalid_;

    double cached_value_;

    void handle_change_();
};
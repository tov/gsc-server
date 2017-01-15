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

    Wt::Signal<>& changed() { return changed_; }

private:
    Wt::WLineEdit* edit_;
    Wt::Signal<> changed_;
};
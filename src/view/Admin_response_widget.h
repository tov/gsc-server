#pragma once

#include "../model/Abstract_evaluation.h"

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>
#include <Wt/WSignal>

class Unit_line_edit;
class Explanation_text_area;

class Admin_response_widget : public Wt::WCompositeWidget
{
public:
    Admin_response_widget(Wt::WContainerWidget* parent = nullptr);

    void load(const Abstract_evaluation*);
    bool save(Abstract_evaluation*);

    Wt::WContainerWidget* buttons() const { return buttons_; }

    Wt::Signal<>& valid() { return valid_; }
    Wt::Signal<>& invalid() { return invalid_; }

private:
    Explanation_text_area* explanation_;
    Unit_line_edit* grade_;

    Wt::WContainerWidget* buttons_;

    Wt::Signal<> valid_;
    Wt::Signal<> invalid_;
};


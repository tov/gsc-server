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

    bool is_valid();
    Wt::Signal<>& changed() { return changed_; }

    Wt::WPushButton* save_button() { return save_button_; }
    Wt::WPushButton* retract_button() { return retract_button_; }

private:
    Explanation_text_area* explanation_;
    Unit_line_edit* grade_;
    Wt::WPushButton* save_button_;
    Wt::WPushButton* retract_button_;

    Wt::Signal<> changed_;
};


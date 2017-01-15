#pragma once

#include "../model/Abstract_evaluation.h"

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>
#include <Wt/WSignal>
#include <Wt/WWidget>

#include <vector>

class Unit_line_edit;
class Explanation_text_area;

class Admin_response_widget : public Wt::WCompositeWidget
{
public:
    Admin_response_widget(Wt::WContainerWidget* parent = nullptr);

    void load(const Abstract_evaluation*);
    bool save(Abstract_evaluation*);

    bool is_valid();
    bool is_saved();

    Wt::WContainerWidget* buttons() const { return buttons_; }

    Wt::Signal<>& changed() { return changed_; }

    void setFocus(bool focus) override;

private:
    const Abstract_evaluation* model_;

    Explanation_text_area* explanation_;
    Unit_line_edit* grade_;

    Wt::WContainerWidget* buttons_;

    Wt::Signal<> changed_;

    void handle_change_();
};


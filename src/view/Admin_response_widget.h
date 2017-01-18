#pragma once

#include "../model/Abstract_evaluation.h"

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>
#include <Wt/WSignal>
#include <Wt/WWidget>

#include <vector>

class Unit_line_edit;
class Explanation_edit_widget;

class Admin_response_widget : public Wt::WCompositeWidget
{
public:
    Admin_response_widget(Wt::WContainerWidget* parent = nullptr);
    Admin_response_widget(Abstract_evaluation*,
                          Wt::WContainerWidget* parent = nullptr);

    void load(const Abstract_evaluation*);
    bool save(Abstract_evaluation*);

    bool is_valid();
    bool is_saved();

    Wt::WContainerWidget* buttons() const { return buttons_; }

    Wt::Signal<>& changed() { return changed_; }

    void setFocus(bool focus) override;

private:
    struct {
        std::string explanation;
        double score;
    } model_;

    void set_model_(const Abstract_evaluation*);

    Explanation_edit_widget* explanation_;
    Unit_line_edit* grade_;

    Wt::WContainerWidget* buttons_;

    Wt::Signal<> changed_;

    void handle_change_();
};


#pragma once

#include "../model/Abstract_evaluation.h"

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>

class Admin_response_widget : public Wt::WCompositeWidget
{
public:
    Admin_response_widget(const Abstract_evaluation*,
                          Wt::WContainerWidget* parent = nullptr);

    void save(Abstract_evaluation*);

private:
    Wt::WTextArea* explanation_;
    Wt::WLineEdit* grade_;
    Wt::WPushButton* save_button_;
    Wt::WPushButton* retract_button_;
    Wt::WText* status_;
};


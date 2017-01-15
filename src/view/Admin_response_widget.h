#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>

class Admin_response_widget : public Wt::WCompositeWidget
{
public:
    Admin_response_widget(Wt::WContainerWidget* parent = nullptr);

private:
    Wt::WTextArea* explanation_;
    Wt::WLineEdit* grade_;
    Wt::WPushButton* save_button_;
    Wt::WPushButton* retract_button_;
    Wt::WText* status_;
};


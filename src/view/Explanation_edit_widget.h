#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WTextArea>

class Explanation_edit_widget : public Wt::WTextArea
{
public:
    Explanation_edit_widget(Wt::WContainerWidget* parent = nullptr);
};

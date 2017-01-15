#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WTextArea>

class Explanation_text_area : public Wt::WTextArea
{
public:
    Explanation_text_area(Wt::WContainerWidget* parent = nullptr);
};

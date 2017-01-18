#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>

#include <string>

class Explanation_view_widget : public Wt::WCompositeWidget {
public:
    Explanation_view_widget(const std::string& content,
                       Wt::WContainerWidget* parent = nullptr);
};
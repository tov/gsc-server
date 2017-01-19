#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>

#include <string>

class File_viewer_widget;

class Explanation_view_widget : public Wt::WCompositeWidget {
public:
    Explanation_view_widget(const std::string& content,
                            Wt::WContainerWidget* parent = nullptr);

    Explanation_view_widget(const std::string& content,
                            File_viewer_widget* viewer,
                            Wt::WContainerWidget* parent = nullptr);
};
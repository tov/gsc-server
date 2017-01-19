#include "Explanation_view_widget.h"
#include "File_viewer_widget.h"

#include <Wt/WText>

Explanation_view_widget::Explanation_view_widget(const std::string& content,
                                                 Wt::WContainerWidget* parent)
        : Explanation_view_widget(content, nullptr, parent)
{ }

Explanation_view_widget::Explanation_view_widget(const std::string& content,
                                                 File_viewer_widget* viewer,
                                                 Wt::WContainerWidget* parent)
        : WCompositeWidget(parent)
{
    setImplementation(new Wt::WText(content, Wt::PlainText));
}
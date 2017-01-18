#include "Explanation_view_widget.h"

#include <Wt/WText>

Explanation_view_widget::Explanation_view_widget(const std::string& content,
                                       Wt::WContainerWidget* parent)
        : WCompositeWidget(parent)
{
    setImplementation(new Wt::WText(content, Wt::PlainText));
}
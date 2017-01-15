#include "Admin_response_widget.h"

Admin_response_widget::Admin_response_widget(Wt::WContainerWidget* parent)
        : WCompositeWidget(parent)
{
    auto impl = new Wt::WContainerWidget;
    setImplementation(impl);


}

#include "Admin_response_widget.h"

Admin_response_widget::Admin_response_widget(const Abstract_evaluation* model,
                                             Wt::WContainerWidget* parent)
        : WCompositeWidget(parent)
{
    auto impl = new Wt::WContainerWidget;
    setImplementation(impl);


}

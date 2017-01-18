#include "Explanation_edit_widget.h"

Explanation_edit_widget::Explanation_edit_widget(Wt::WContainerWidget* parent)
        : Wt::WTextArea(parent)
{
    setInline(false);
    setStyleClass("explanation");
}

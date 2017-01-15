#include "Explanation_text_area.h"

Explanation_text_area::Explanation_text_area(Wt::WContainerWidget* parent)
        : Wt::WTextArea(parent)
{
    setInline(false);
    setStyleClass("explanation");
}

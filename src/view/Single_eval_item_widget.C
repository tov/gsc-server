#include "Single_eval_item_widget.h"

Single_eval_item_widget::Single_eval_item_widget(
        const Submission::Item& model,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Base_eval_item_widget(model, main, session, parent)
{
    add_item_heading_();
    add_question_();
}


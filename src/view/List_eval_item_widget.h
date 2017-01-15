#pragma once

#include "Base_eval_item_widget.h"

class List_eval_item_widget : public Base_eval_item_widget
{
public:
    List_eval_item_widget(const Submission::Item&,
                          Evaluation_view&,
                          Session&,
                          Wt::WContainerWidget* parent = nullptr);

private:
    void focus_action_();

    void add_scores_();
    void add_buttons_();
};


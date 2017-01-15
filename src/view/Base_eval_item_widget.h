#pragma once

#include "../model/Eval_item.h"
#include "../model/Submission.h"

#include <Wt/WContainerWidget>

class Evaluation_view;
class Session;

class Base_eval_item_widget : public Wt::WContainerWidget
{
public:
    Base_eval_item_widget(const Submission::Item&,
                          Evaluation_view&,
                          Session&,
                          Wt::WContainerWidget* parent = nullptr);

protected:
    const Submission::Item& model_;
    Evaluation_view& main_;
    Session& session_;

    void retract_action_();

    void add_item_heading_();
    void add_question_();
    void add_evaluation_(const std::string& heading,
                         const std::string& score,
                         const std::string& explanation);
};

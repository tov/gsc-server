#pragma once

#include "Abstract_file_view.h"
#include "../model/auth/User.h"
#include "../model/Submission.h"
#include "../model/Eval_item.h"

#include <Wt/WContainerWidget>
#include <Wt/Dbo/ptr>

#include <vector>

class Eval_item_widget;

class Evaluation_view : public Abstract_file_view
{
public:
    Evaluation_view(const Wt::Dbo::ptr<Submission>&,
                    Session&,
                    Wt::WContainerWidget* parent = nullptr);

    void go_to(unsigned int);
    void go_default();

    // For evaluation_list_view_item
    const dbo::ptr<Submission>& submission() const { return submission_; }

    // Is the current user allowed to change the self evaluation right now?
    bool can_eval();

private:
    void load_();

    friend class Eval_item_widget;
};
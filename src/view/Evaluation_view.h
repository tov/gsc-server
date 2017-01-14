#pragma once

#include "Abstract_file_view.h"
#include "../model/auth/User.h"
#include "../model/Submission.h"
#include "../model/Eval_item.h"

#include <Wt/WContainerWidget>
#include <Wt/Dbo/ptr>

#include <vector>

class Eval_widget;

class Evaluation_view : public Abstract_file_view
{
public:
    Evaluation_view(const Wt::Dbo::ptr<Submission>&,
                    Session&,
                    Wt::WContainerWidget* parent = nullptr);

    void go_to(unsigned int);
    void go_default();

private:
    User::Role role_;

    std::vector<std::unique_ptr<Wt::WWidget>> rows_;

    // Is the current user allowed to change the self evaluation right now?
    bool can_eval_();

    void load_();

    friend class Eval_widget;
};
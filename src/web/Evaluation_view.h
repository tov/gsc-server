#pragma once

#include "Abstract_file_view.h"

#include <Wt/WContainerWidget.h>
#include <Wt/Dbo/ptr.h>
#include <Wt/WSignal.h>

class Submission;

class Evaluation_view : public Abstract_file_view
{
public:
    Evaluation_view(const Wt::Dbo::ptr<Submission>&, Session&);

    void go_to(unsigned int);
    void go_default();

    // For evaluation_list_view_item
    const Wt::Dbo::ptr<Submission>& submission() const { return submission_; }

    // Is the current user allowed to change the self evaluation right now?
    bool can_eval();

private:
    Wt::Signal<> dummy_;

    void load_();
};
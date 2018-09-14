#pragma once

#include <Wt/Dbo/ptr.h>
#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>

class Session;
class Submission;

class Submission_owner_widget : public Wt::WCompositeWidget
{
public:
    Submission_owner_widget(const Wt::Dbo::ptr<Submission> &,
                            Session &,
                            Wt::Signal<> &changed);

private:
    Session& session_;
    Wt::Dbo::ptr<Submission> submission_;
    Wt::WContainerWidget* impl_;
    Wt::Signal<>& changed_;

    void update_();
    void update_admin_();
    void update_grader_();
    void update_student_();

    void break_up_partnership_();
};
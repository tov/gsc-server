#pragma once

#include <Wt/Dbo/ptr>
#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>

class Session;
class Submission;

class Submission_owner_widget : public Wt::WCompositeWidget
{
public:
    Submission_owner_widget(const Wt::Dbo::ptr<Submission>&,
                            Session&,
                            Wt::WContainerWidget* parent = nullptr);

private:
    Session& session_;
    Wt::Dbo::ptr<Submission> submission_;
    Wt::WContainerWidget* impl_;

    void update_();
    void update_admin_();
    void update_grader_();
    void update_student_();
};
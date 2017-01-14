#pragma once

#include "../model/auth/User.h"
#include "../model/Submission.h"

#include <Wt/WContainerWidget>

class Abstract_widget_factory;
class Evaluation_view;
class Session;

class Eval_widget : public Wt::WContainerWidget
{
public:
    Eval_widget(const Submission::Item&, bool is_singular,
                Evaluation_view&,
                Session&,
                Wt::WContainerWidget* parent = nullptr);

    static std::unique_ptr<Eval_widget>
    create(const Submission::Item&, bool is_singular,
           Evaluation_view&, Session&,
           Wt::WContainerWidget* parent = nullptr);

protected:
    const Submission::Item& model_;
    bool is_singular_;
    Session& session_;
    Evaluation_view& main_;

    std::unique_ptr<Abstract_widget_factory> self_factory_;
    std::unique_ptr<Abstract_widget_factory> grader_factory_;

    Wt::WContainerWidget* self_area_;
    Wt::WContainerWidget* self_buttons_;
    Wt::WContainerWidget* grader_area_;
    Wt::WContainerWidget* grader_buttons_;

    bool can_eval() const;
    bool can_grade() const;
    bool can_admin() const;
    User::Role role() const;

    virtual double score() const = 0;
    virtual void set_score(double) = 0;
    virtual std::string explanation() const = 0;
    virtual void set_explanation(const std::string&) = 0;
    virtual void reset() = 0;

    void load_();
    void save_();
    void retract_();

    void defocus_action_();
    void save_next_action_();
    void save_action_();
    void retract_action_();
    void focus_action_();
};


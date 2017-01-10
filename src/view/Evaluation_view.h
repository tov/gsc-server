#pragma once

#include "../model/auth/User.h"

#include <Wt/WContainerWidget>
#include <Wt/Dbo/ptr>

#include <vector>

class Session;
class Submission;
class Eval_item;
class Self_eval;
class Grader_eval;

class Eval_widget;
class File_viewer_widget;

struct Evaluation_view_row_model
{
    dbo::ptr<Eval_item> eval_item;
    dbo::ptr<Self_eval> self_eval;
    dbo::ptr<Grader_eval> grader_eval;
};

class Evaluation_view : public Wt::WContainerWidget
{
public:
    Evaluation_view(const Wt::Dbo::ptr<Submission>&,
                    Session&,
                    Wt::WContainerWidget* parent = nullptr);

    void go_to(unsigned int);
    void go_default();

private:
    Wt::Dbo::ptr<Submission> submission_;
    Session& session_;
    User::Role role_;

    using Row_model = Evaluation_view_row_model;

    std::vector<Row_model> model_;
    double total_value_;

    File_viewer_widget* viewer_;
    Wt::WContainerWidget* right_column_;

    std::vector<std::unique_ptr<Eval_widget>> rows_;

    // Is the current user allowed to change the self evaluation right now?
    bool can_eval_();

    void load_();

    friend class Eval_widget;
};
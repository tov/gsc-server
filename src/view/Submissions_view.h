#pragma once

#include "../model/Submission.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>

#include <memory>

class Exam_grade;
class Session;
class User;

struct Submissions_view_model_item {
    Wt::Dbo::ptr<Submission> submission;
    size_t file_count = 0;
    Submission::Eval_status eval_status;
};

struct Submissions_view_model {
    std::vector<Submissions_view_model_item> submissions;
    std::vector<Wt::Dbo::ptr<Exam_grade>> exams;
};

class Submissions_view_row : public Wt::WObject
{
public:
    enum columns { NAME, STATUS, DUE_DATE, EVAL_DATE, GRADE, ACTION, };

    static std::unique_ptr<Submissions_view_row>
    construct(const Submissions_view_model_item& model,
              Session& session,
              Wt::WTableRow* row);

    static void add_headings(Wt::WTableRow*);

protected:
    Submissions_view_row(const Submissions_view_model_item& model,
                         Session& session,
                         Wt::WTableRow* row);

    const Submissions_view_model_item& model_;
    Session& session_;
    Wt::WTableRow* row_;

    virtual void update();

    virtual void set_files_action(const char[]);
    virtual void set_eval_action(const char[]);
    virtual void set_action_style_class(const char[]);

private:
    Wt::WText* status_;
    Wt::WText* grade_;
    Wt::WPushButton* action_;
    std::string action_url_;

    void action();
};

class Submissions_view : public Wt::WContainerWidget
{
public:
    Submissions_view(const Wt::Dbo::ptr<User>&, Session&);

private:
    Session& session_;
    std::unique_ptr<Submissions_view_model> model_;
    std::vector<std::unique_ptr<Submissions_view_row>> rows_;
    Wt::Signal<> dummy_;
};
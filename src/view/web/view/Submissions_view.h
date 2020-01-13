#pragma once

#include "../Submission_context.h"
#include "../../../model/Submission.h"
#include "../../../common/autovector.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>

#include <memory>

class Exam_grade;
class Session;
class User;

struct Submissions_view_model {
    struct Item;

    auto_vector<Item>                      submissions;
    std::vector<Wt::Dbo::ptr<Exam_grade>>  exams;
    Wt::Dbo::ptr<User>                     principal;

    Submissions_view_model(Wt::Dbo::ptr<User> const& principal, Session&);
};

struct Submissions_view_model::Item {
    Item() = default;
    Item(Wt::Dbo::ptr<Submission> const&);

    Wt::Dbo::ptr<Submission> submission;
    size_t file_count = 0;
    Submission::Eval_status eval_status = Submission::Eval_status::empty;
};

class Submissions_view_row : public Wt::WObject
{
public:
    enum columns { NAME, STATUS, DUE_DATE, EVAL_DATE, GRADE, ACTION, };

    using View_model = Submissions_view_model::Item;

    static std::unique_ptr<Submissions_view_row>
    construct(View_model const& model,
              Session& session,
              Wt::WTableRow* row);

    static void add_headings(Wt::WTableRow*);

protected:
    Submissions_view_row(View_model const& model,
                         Session& session,
                         Wt::WTableRow* row);

    View_model const& model_;
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
    void reload_();
    void on_change_(Submission_change_message);

    Session& session_;
    Submissions_view_model model_;
    std::vector<std::unique_ptr<Submissions_view_row>> rows_;
    Submission_change_signal changed_;
};
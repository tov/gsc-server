#pragma once

#include "../Submission_context.h"
#include "../../../Navigate.h"
#include "../../../model/Submission.h"
#include "../../../common/autovector.h"

#include <Wt/Dbo/Dbo.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>

#include <memory>

class Exam_grade;

class Session;

class User;

class Submissions_view : public Wt::WContainerWidget
{
public:
    using Status      = Submission::Status;
    using Eval_status = Submission::Eval_status;

    Submissions_view(const Wt::Dbo::ptr<User>&, Session&);

    struct Model
    {
        struct Item
        {
            Item() = default;
            Item(Wt::Dbo::ptr<Submission> const& submission,
                 Wt::Dbo::ptr<User> const& principal);

            Wt::Dbo::ptr<Submission> submission;
            size_t file_count       = 0;
            Eval_status eval_status = Eval_status::empty;

            Wt::Dbo::ptr<User>       principal;
        };

        auto_vector<Item>                     submissions;
        std::vector<Wt::Dbo::ptr<Exam_grade>> exams;
        Wt::Dbo::ptr<User>                    principal;

        Model(Wt::Dbo::ptr<User> const& principal, Session&);
    };

    class Row_view;

private:
    Session& session_;
    Model                                  model_;
    std::vector<std::unique_ptr<Row_view>> rows_;
    Submission_change_signal               changed_;

    void reload_();
    void on_change_(Submission_change_message);
};

class Submissions_view::Row_view : public Wt::WObject
{
public:
    enum columns
    {
        NAME, STATUS, DUE_DATE, EVAL_DATE, GRADE, FILES, EVAL,
    };

    using Row_model = Submissions_view::Model::Item;

    static std::unique_ptr<Row_view>
    construct(Row_model const& model,
              Session& session,
              Wt::WTableRow* row);

    static void add_headings(Wt::WTableRow*);

protected:
    Row_view(Row_model const& model,
             Session& session,
             Wt::WTableRow* row);

    Row_model const& model_;
    Session& session_;
    Wt::WTableRow* row_;

    virtual void update();

    virtual void set_files_button(const char* title, const char* style);
    virtual void set_eval_button(const char* title, const char* style);

    Wt::Dbo::ptr<Submission> const& submission() const
    { return model_.submission; }

    Wt::Dbo::ptr<Assignment> const& assignment() const
    { return submission()->assignment(); }

private:
    Wt::WText      * status_;
    Wt::WText      * grade_;
    Wt::WPushButton* files_btn_;
    Wt::WPushButton* eval_btn_;

    void update_styles_();
    void update_status_();
    void update_score_();
    void update_buttons_();

    Wt::WString get_row_style_() const;

    Navigate go_files_() const;
    Navigate go_eval_() const;
};


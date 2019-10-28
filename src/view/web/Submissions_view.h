#pragma once

#include "Submission_context.h"
#include "../../model/Submission.h"

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

template <class T>
class auto_vector : public std::vector<T>
{
public:
    using Base = std::vector<T>;
    using typename Base::vector;
    using typename Base::value_type;
    using typename Base::difference_type;
    using typename Base::reference;
    using typename Base::const_reference;
    using typename Base::iterator;
    using typename Base::const_iterator;
    using typename Base::reverse_iterator;
    using typename Base::const_reverse_iterator;

    reference at(size_t ix)
    {
        if (!in_bounds_(ix))
            Base::resize(ix + 1);

        return Base::operator[](ix);
    }

    reference operator[](size_t ix)
    {
        return at(ix);
    }

    const_reference at(size_t ix) const
    {
        if (in_bounds_(ix))
            return Base::operator[](ix);
        else
            return value_type{};
    }

    const_reference operator[](size_t ix) const
    {
        return at(ix);
    }

private:
    bool in_bounds_(size_t ix) const
    {
        return ix < Base::size();
    }
};

struct Submissions_view_model {
    auto_vector<Submissions_view_model_item> submissions;
    std::vector<Wt::Dbo::ptr<Exam_grade>>    exams;
    Wt::Dbo::ptr<User>                       principal;

    Submissions_view_model(Wt::Dbo::ptr<User> const& principal, Session&);
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
    void reload_();
    void on_change_(Submission_change_message);

    Session& session_;
    Submissions_view_model model_;
    std::vector<std::unique_ptr<Submissions_view_row>> rows_;
    Submission_change_signal changed_;
};
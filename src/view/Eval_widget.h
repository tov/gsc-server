#pragma once

#include <Wt/WContainerWidget>

class Evaluation_view;
class Evaluation_view_row_model;
class Session;

class Eval_widget : public Wt::WContainerWidget
{
public:
    enum class Mode {
        self_eval,
        self_view,
        grader_eval,
        admin_view,
    };

    using Row_model = Evaluation_view_row_model;

    Eval_widget(Row_model&, Mode,
                Evaluation_view&,
                Session&,
                Wt::WContainerWidget* parent = nullptr);

    static std::unique_ptr<Eval_widget>
    create(Row_model&, Mode, Evaluation_view&, Session&,
           Wt::WContainerWidget* parent = nullptr);

protected:
    Row_model& model_;
    Mode mode_;
    Session& session_;
    Evaluation_view& main_;

    Wt::WContainerWidget* response_;

    virtual double score() const = 0;
    virtual void set_score(double) = 0;
    virtual std::string explanation() const = 0;
    virtual void set_explanation(const std::string&) = 0;
    virtual void reset() = 0;

    void load_();
    void save_();
    void retract_();
};


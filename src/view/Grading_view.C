#include "Grading_view.h"
#include "../Navigate.h"
#include "../model/Self_eval.h"
#include "../model/Eval_item.h"
#include "../model/Grader_eval.h"
#include "../model/Assignment.h"
#include "../model/Session.h"

#include <Wt/Dbo/ptr>
#include <Wt/WApplication>
#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WSignal>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WTextArea>

class Abstract_grading_widget : public Wt::WCompositeWidget
{
public:
    Abstract_grading_widget(const dbo::ptr<Grader_eval>&,
                            Session&,
                            Wt::WContainerWidget* parent = nullptr);

    static Abstract_grading_widget*
    create(const dbo::ptr<Grader_eval>&,
           Session&,
           Wt::WContainerWidget* parent = nullptr);

    Wt::Signal<>& hold_signal() { return hold_signal_; }
    Wt::Signal<double>& save_signal() { return save_signal_; }

    virtual ~Abstract_grading_widget();

protected:
    dbo::ptr<Grader_eval> model_;
    Session& session_;
    Wt::WContainerWidget* const buttons_;

    void add_hold_button();

private:
    Wt::Signal<> hold_signal_;
    Wt::Signal<double> save_signal_;

    void hold_action_();
};

class Boolean_grading_widget : public Abstract_grading_widget
{
public:
    Boolean_grading_widget(const dbo::ptr<Grader_eval>&,
                           Session&,
                           Wt::WContainerWidget* parent = nullptr);

private:
    void yes_action_();
    void no_action_();
};

class Scale_grading_widget : public Abstract_grading_widget
{
public:
    Scale_grading_widget(const dbo::ptr<Grader_eval>&,
                         Session&,
                         Wt::WContainerWidget* parent = nullptr);

private:
    Wt::WLineEdit* edit_;

    void apply_action_();
};

Abstract_grading_widget*
Abstract_grading_widget::create(const dbo::ptr <Grader_eval>& grader_eval,
                                Session& session,
                                Wt::WContainerWidget* parent)
{
    auto type = grader_eval->self_eval()->eval_item()->type();
    if (type == Eval_item::Type::Boolean)
        return new Boolean_grading_widget(grader_eval, session, parent);
    else
        return new Scale_grading_widget(grader_eval, session, parent);
}

Abstract_grading_widget::Abstract_grading_widget(
        const dbo::ptr<Grader_eval>& model,
        Session& session,
        Wt::WContainerWidget* parent)
        : WCompositeWidget(parent),
          model_(model),
          session_(session),
          buttons_(new Wt::WContainerWidget)
{
    buttons_->setStyleClass("buttons");
    setImplementation(buttons_);
}

void Abstract_grading_widget::add_hold_button()
{
    auto hold_button = new Wt::WPushButton("Hold", buttons_);
    hold_button->clicked().connect(this,
                                   &Abstract_grading_widget::hold_action_);
}

Abstract_grading_widget::~Abstract_grading_widget()
{
    if (model_->status() == Grader_eval::Status::editing) {
        dbo::Transaction transaction(session_);
        model_.remove();
    }
}

void Abstract_grading_widget::hold_action_()
{
    hold_signal().emit();
}

Boolean_grading_widget::Boolean_grading_widget(
        const dbo::ptr<Grader_eval>& model,
        Session& session,
        Wt::WContainerWidget* parent)
        : Abstract_grading_widget(model, session, parent)
{
    auto yes_button = new Wt::WPushButton("Yes", buttons_);
    yes_button->clicked().connect(this, &Boolean_grading_widget::yes_action_);
    auto no_button = new Wt::WPushButton("No", buttons_);
    no_button->clicked().connect(this, &Boolean_grading_widget::no_action_);

    add_hold_button();
}

void Boolean_grading_widget::yes_action_()
{
    save_signal().emit(1.0);
}

void Boolean_grading_widget::no_action_()
{
    save_signal().emit(0.0);
}

Scale_grading_widget::Scale_grading_widget(
        const dbo::ptr<Grader_eval>& model,
        Session& session,
        Wt::WContainerWidget* parent)
        : Abstract_grading_widget(model, session, parent)
{
    edit_ = new Wt::WLineEdit(buttons_);
    edit_->setEmptyText("[0.0, 1.0]");

    auto apply_button = new Wt::WPushButton("Apply", buttons_);
    apply_button->clicked().connect(this,
                                    &Scale_grading_widget::apply_action_);

    add_hold_button();
}

void Scale_grading_widget::apply_action_()
{
    std::istringstream iss(edit_->text().toUTF8());
    double score;

    if (iss >> score && score >= 0 && score <= 1) {
        save_signal().emit(score);
    } else {
        edit_->setText("");
        edit_->setFocus();
    }
}

Grading_view::Grading_view(const Wt::Dbo::ptr<Self_eval> self_eval,
                           Session& session,
                           Wt::WContainerWidget* parent)
        : Abstract_file_view(self_eval->submission(), session, parent),
          model_(self_eval)
{
    auto widget = new Wt::WTemplate(
        "<h4>Question ${sequence} <small>${homework}</small></h4>"
        "<p>${question}</p>"
        "<p class='answer'>${self_grade}</p>"
        "<p class='explanation'>${self_explanation}</p>"
        "${grader_explanation}"
        "${grading_widget}"
        "<p class='status'>Status: ${status}</p>",
        right_column_
    );

    dbo::Transaction transaction(session_);
    auto eval_item = self_eval->eval_item();
    auto assignment = eval_item->assignment();

    auto grader_eval = self_eval->grader_eval();
    if (!grader_eval) {
        grader_eval = session_.add(new Grader_eval(self_eval,
                                                   session_.user()));
    }

    std::string sequence =
            boost::lexical_cast<std::string>(eval_item->sequence());
    auto grader_explanation = new Wt::WTextArea(grader_eval->evaluation());
    auto grading_widget = Abstract_grading_widget::create(grader_eval,
                                                          session_);

    std::ostringstream status;
    switch (grader_eval->status()) {
        case Grader_eval::Status::editing:
            status << "unsaved";
            break;
        case Grader_eval::Status::held_back:
            status << "held";
            break;
        case Grader_eval::Status::ready:
            status << "saved (";
            status << Eval_item::pct_string(grader_eval->score());
            status << ")";
            break;
    }

    widget->bindWidget("sequence", new Wt::WText(sequence));
    widget->bindWidget("homework", new Wt::WText(assignment->name()));
    widget->bindWidget("question", new Wt::WText(eval_item->prompt()));
    widget->bindWidget("self_grade",
                       new Wt::WText(eval_item->format_score(self_eval->score())));
    widget->bindWidget("self_explanation",
                       new Wt::WText(self_eval->explanation()));
    widget->bindWidget("grader_explanation", grader_explanation);
    widget->bindWidget("grading_widget", grading_widget);
    widget->bindWidget("status", new Wt::WText(status.str()));

    grader_explanation->setStyleClass("explanation");
    grader_explanation->setInline(false);
}


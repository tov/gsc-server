#include "Eval_widget.h"
#include "Evaluation_view.h"
#include "Widget_factory.h"
#include "../model/Eval_item.h"
#include "../model/Grader_eval.h"
#include "../model/Self_eval.h"
#include "../model/Session.h"

#include <Wt/WBreak>
#include <Wt/WButtonGroup>
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WRadioButton>
#include <Wt/WText>
#include <Wt/WTextArea>

#include <iomanip>

Eval_widget::Eval_widget(
        Row_model& model,
        bool is_singular,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : WContainerWidget(parent),
          model_(model),
          is_singular_(is_singular),
          main_(main),
          session_(session)
{
    setStyleClass("edit-widget");

    if (can_eval() && is_singular_)
        self_factory_ = make_editable_widget_factory();
    else
        self_factory_ = make_viewable_widget_factory();

    if (can_grade() && is_singular_)
        grader_factory_ = make_editable_widget_factory();
    else
        grader_factory_ = make_viewable_widget_factory();

    auto pct = 100 * model.eval_item->relative_value() / main_.total_value_;
    std::ostringstream title;
    title << "<h4>Question " << model.eval_item->sequence();
    title << " <small>(" << std::setprecision(2) << pct << "%)</small>";
    title << "</h4>";
    new Wt::WText(title.str(), this);

    auto prompt = new Wt::WText(model_.eval_item->prompt(), this);
    prompt->setInline(false);

    self_area_ = new Wt::WContainerWidget(this);

    self_buttons_ = new Wt::WContainerWidget(this);
    self_buttons_->setStyleClass("buttons");

    if (model_.grader_eval || grader_factory_->is_editable()) {
        new Wt::WText("<h5>Grader evaluation</h5>", this);
        grader_area_ = new Wt::WContainerWidget(this);
    } else {
        grader_area_ = nullptr;
    }

    grader_buttons_ = new Wt::WContainerWidget(this);
    grader_buttons_->setStyleClass("buttons");
}

void Eval_widget::load_()
{
    if (model_.self_eval) {
        set_score(model_.self_eval->score());
        set_explanation(model_.self_eval->explanation());
    } else reset();
}

void Eval_widget::save_()
{
    if (!can_eval()) return;

    dbo::Transaction transaction(session_);

    if (!model_.self_eval)
        model_.self_eval
                = session_.add(new Self_eval(model_.eval_item,
                                             main_.submission_));

    auto self_eval = model_.self_eval.modify();
    self_eval->set_score(score());
    self_eval->set_explanation(explanation());
}

void Eval_widget::retract_()
{
    if (!can_eval()) return;

    dbo::Transaction transaction(session_);
    model_.self_eval.remove();
    model_.grader_eval.remove();
    transaction.commit();

    model_.self_eval = dbo::ptr<Self_eval>();
    model_.grader_eval = dbo::ptr<Grader_eval>();
}

void Eval_widget::defocus_action_()
{
    main_.go_default();
}

void Eval_widget::save_next_action_()
{
    save_();

    int next = model_.self_eval->eval_item()->sequence() + 1;
    if (0 < next && next < main_.model_.size())
        main_.go_to((unsigned int) next);
    else
        main_.go_default();
}

void Eval_widget::save_action_()
{
    save_();
    main_.go_default();
}

void Eval_widget::retract_action_()
{
    retract_();
    focus_action_();
}

void Eval_widget::focus_action_()
{
    main_.go_to((unsigned int) model_.eval_item->sequence());
}

bool Eval_widget::can_eval() const
{
    return main_.can_eval_();
}

bool Eval_widget::can_grade() const
{
    return session_.user()->can_grade();
}

User::Role Eval_widget::role() const
{
    return main_.role_;
}

class Response_eval_widget : public Eval_widget
{
public:
    Response_eval_widget(Row_model&, bool, Evaluation_view&, Session&,
                         WContainerWidget* parent = nullptr);

protected:
    virtual std::string explanation() const override;
    virtual void set_explanation(const std::string& string) override;
    virtual void reset() override;

    // Allow derived classes to populate this:
    Wt::WContainerWidget* score_holder_;
    // Allow derived classes to hide this:
    Abstract_explanation_holder* explanation_holder_;
};

Response_eval_widget::Response_eval_widget(
        Row_model& model,
        bool is_singular,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Eval_widget(model, is_singular, main, session, parent)
{
    score_holder_ = new Wt::WContainerWidget(self_area_);
    score_holder_->setStyleClass("score");
    explanation_holder_ = self_factory_->make_explanation_holder(self_area_);

    if (self_factory_->is_editable()) {
        auto save = new Wt::WPushButton("Save", self_buttons_);
        save->clicked().connect(this, &Response_eval_widget::save_next_action_);
    } else if (can_eval()) {
        auto edit = new Wt::WPushButton("Edit", self_buttons_);
        edit->clicked().connect(this, &Response_eval_widget::focus_action_);
    } else if (is_singular_) {
        auto back = new Wt::WPushButton("Back", self_buttons_);
        back->clicked().connect(this, &Response_eval_widget::defocus_action_);
    } else {
        auto edit = new Wt::WPushButton("View", self_buttons_);
        edit->clicked().connect(this, &Response_eval_widget::focus_action_);
    }
}

std::string Response_eval_widget::explanation() const
{
    return explanation_holder_->explanation();
}

void
Response_eval_widget::set_explanation(const std::string& text)
{
    explanation_holder_->set_explanation(text);
}

void Response_eval_widget::reset()
{
    explanation_holder_->set_explanation("");
}

class Boolean_eval_widget : public Response_eval_widget
{
public:
    Boolean_eval_widget(Row_model&, bool, Evaluation_view&, Session&,
                        WContainerWidget* parent = nullptr);

protected:
    virtual double score() const override;
    virtual void set_score(double d) override;
    virtual void reset() override;

private:
    Abstract_boolean_option* boolean_option_;
    void toggle_explanation_();
};

Boolean_eval_widget::Boolean_eval_widget(
        Row_model& model,
        bool is_singular,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Response_eval_widget(model, is_singular, main, session, parent)
{
    boolean_option_ = self_factory_->make_boolean_option(score_holder_);
    explanation_holder_->hide();
    boolean_option_->changed().connect(this,
                                      &Boolean_eval_widget::toggle_explanation_);

    load_();
}

double Boolean_eval_widget::score() const
{
    return boolean_option_->value() ? 1.0 : 0.0;
}

void Boolean_eval_widget::set_score(double d)
{
    boolean_option_->set_value(d > 0);
    toggle_explanation_();
}

void Boolean_eval_widget::reset()
{
    Response_eval_widget::reset();
    boolean_option_->clear();
}

void Boolean_eval_widget::toggle_explanation_()
{
    if (boolean_option_->value())
        explanation_holder_->show();
    else
        explanation_holder_->hide();
}

class Scale_eval_widget
        : public Response_eval_widget
{
public:
    Scale_eval_widget(Row_model&, bool, Evaluation_view&, Session&,
                      WContainerWidget* parent = nullptr);

protected:
    virtual double score() const override;
    virtual void set_score(double d) override;
    virtual void reset() override;

private:
    Abstract_unit_scale* unit_scale_;
};

Scale_eval_widget::Scale_eval_widget(
        Row_model& model,
        bool is_singular,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Response_eval_widget(model, is_singular, main, session, parent)
{
    unit_scale_ = self_factory_->make_unit_scale(score_holder_);
    load_();
}

double Scale_eval_widget::score() const
{
    return unit_scale_->value();
}

void Scale_eval_widget::set_score(double d)
{
    unit_scale_->set_value(d);
}

void Scale_eval_widget::reset()
{
    Response_eval_widget::reset();
    unit_scale_->clear();
}

class Informational_eval_widget : public Eval_widget
{
public:
    Informational_eval_widget(Row_model&, bool, Evaluation_view&, Session&,
                              WContainerWidget* parent = nullptr);

protected:
    virtual double score() const override;
    virtual void set_score(double d) override;
    virtual std::string explanation() const override;
    virtual void set_explanation(const std::string& string) override;
    virtual void reset() override;
};

Informational_eval_widget::Informational_eval_widget(
        Row_model& model,
        bool is_singular,
        Evaluation_view& main,
        Session& session,
        Wt::WContainerWidget* parent)
        : Eval_widget(model, is_singular, main, session, parent)
{
    load_();

    if (self_factory_->is_editable()) {
        auto save = new Wt::WPushButton("Okay", self_buttons_);
        save->clicked().connect(this,
                                &Informational_eval_widget::save_next_action_);
    } else if (can_eval()) {
        auto edit = new Wt::WPushButton("Edit", self_buttons_);
        edit->clicked().connect(this,
                                &Informational_eval_widget::focus_action_);
    } else if (is_singular_) {
        auto back = new Wt::WPushButton("Back", self_buttons_);
        back->clicked().connect(this, &Informational_eval_widget::defocus_action_);
    } else {
        auto edit = new Wt::WPushButton("View", self_buttons_);
        edit->clicked().connect(this, &Informational_eval_widget::focus_action_);
    }
}

double Informational_eval_widget::score() const
{
    return 1;
}

void Informational_eval_widget::set_score(double)
{
    // no-op
}

std::string Informational_eval_widget::explanation() const
{
    return "";
}

void
Informational_eval_widget::set_explanation(const std::string&)
{
    // no-op
}

void Informational_eval_widget::reset()
{
    // no-op
}

std::unique_ptr<Eval_widget>
Eval_widget::create(Row_model& model, bool is_singular, Evaluation_view& main,
                    Session& session, Wt::WContainerWidget* parent)
{
    switch (model.eval_item->type()) {
        case Eval_item::Type::Boolean:
            return std::make_unique<Boolean_eval_widget>(model, is_singular,
                                                         main, session, parent);
        case Eval_item::Type::Scale:
            return std::make_unique<Scale_eval_widget>(model, is_singular, main,
                                                       session, parent);
        case Eval_item::Type::Informational:
            return std::make_unique<Informational_eval_widget>(model,
                                                               is_singular,
                                                               main,
                                                               session, parent);
    }
}



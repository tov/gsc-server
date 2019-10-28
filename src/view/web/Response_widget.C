#include "Response_widget.h"

#include <Wt/WButtonGroup.h>
#include <Wt/WRadioButton.h>
#include <Wt/WSlider.h>
#include <Wt/WTextArea.h>

Response_widget::Response_widget()
{
    container_ = setNewImplementation<Wt::WContainerWidget>();
}

class Explanation_response_widget : public Response_widget
{
public:
    Explanation_response_widget();

    virtual std::string explanation() const override;
    virtual void set_explanation(const std::string& string) override;

protected:
    Wt::WContainerWidget* score_;
    Wt::WContainerWidget* explanation_holder_;
    Wt::WTextArea* explanation_;
};

Explanation_response_widget::Explanation_response_widget()
        : Response_widget()
{
    score_ = container_->addNew<Wt::WContainerWidget>();
    score_->setStyleClass("score");

    explanation_holder_ = container_->addNew<Wt::WContainerWidget>();

    explanation_holder_->addNew<Wt::WText>(
            "<p>Explain (using line references like <em>L14</em>):</p>");

    explanation_ = explanation_holder_->addNew<Wt::WTextArea>();
    explanation_->setStyleClass("explanation");
    explanation_->setInline(false);
    explanation_->changed().connect(
            this, &Explanation_response_widget::signal_change);
    explanation_->keyWentUp().connect(
            this, &Explanation_response_widget::signal_change);
}

std::string Explanation_response_widget::explanation() const
{
    return explanation_->text().toUTF8();
}

void Explanation_response_widget::set_explanation(const std::string& s)
{
    explanation_->setText(s);
}

class Boolean_response_widget : public Explanation_response_widget
{
public:
    Boolean_response_widget();

    virtual bool is_ready() const override;
    virtual double value() const override;
    virtual void set_value(double d) override;

private:
    std::shared_ptr<Wt::WButtonGroup> no_yes_;
    Wt::WRadioButton* no_;
    Wt::WRadioButton* yes_;

    void toggle_explanation_();
};

Boolean_response_widget::Boolean_response_widget()
{
    no_yes_ = std::make_shared<Wt::WButtonGroup>();
    no_     = score_->addNew<Wt::WRadioButton>("No");
    yes_    = score_->addNew<Wt::WRadioButton>("Yes");
    no_yes_->addButton(no_);
    no_yes_->addButton(yes_);

    no_->setFocus();

    no_yes_->checkedChanged().connect(
            this, &Boolean_response_widget::toggle_explanation_);
    no_yes_->checkedChanged().connect(
            this, &Boolean_response_widget::signal_change);

    toggle_explanation_();
}

bool Boolean_response_widget::is_ready() const
{
    return no_yes_->checkedButton() == no_
           || (no_yes_->checkedButton() == yes_
               && !explanation_->text().empty());
}

double Boolean_response_widget::value() const
{
    return no_yes_->checkedButton() == yes_ ? 1.0 : 0.0;
}

void Boolean_response_widget::set_value(double d)
{
    no_yes_->setCheckedButton(d == 1.0 ? yes_ : no_);
    toggle_explanation_();
}

void Boolean_response_widget::toggle_explanation_()
{
    explanation_holder_->setHidden(value() == 0.0);
}

class Scale_response_widget : public Explanation_response_widget
{
public:
    Scale_response_widget();

    virtual bool is_ready() const override;
    virtual double value() const override;
    virtual void set_value(double d) override;

private:
    Wt::WSlider* slider_;
    bool is_ready_ = false;

    void slider_change_();
};

Scale_response_widget::Scale_response_widget()
{
    slider_ = score_->addNew<Wt::WSlider>();
    slider_->resize(200, 50);
    slider_->setTickPosition(Wt::WSlider::TickPosition::TicksAbove);
    slider_->setTickInterval(20);
    slider_->setMinimum(0);
    slider_->setMaximum(100);
    slider_->setValue(50);

    slider_->valueChanged().connect(this,
                                    &Scale_response_widget::slider_change_);
}

bool Scale_response_widget::is_ready() const
{
    return is_ready_;
}

double Scale_response_widget::value() const
{
    return slider_->value() / 100.0;
}

void Scale_response_widget::set_value(double d)
{
    slider_->setValue(int(100 * d));
    is_ready_ = true;
}

void Scale_response_widget::slider_change_()
{
    is_ready_ = true;
    signal_change();
}

class Informational_response_widget : public Response_widget
{
public:
    Informational_response_widget();

    virtual bool is_ready() const override;

    virtual double value() const override;
    virtual void set_value(double) override;
    virtual std::string explanation() const override;
    virtual void set_explanation(const std::string&) override;
};

Informational_response_widget::Informational_response_widget()
        : Response_widget()
{ }

bool Informational_response_widget::is_ready() const
{ return true; }

double Informational_response_widget::value() const
{ return 1.0; }

void Informational_response_widget::set_value(double)
{ }

std::string Informational_response_widget::explanation() const
{ return ""; }

void Informational_response_widget::set_explanation(const std::string&)
{ }

std::unique_ptr<Response_widget>
Response_widget::create(Eval_item::Type type)
{
    switch (type) {
        case Eval_item::Type::Boolean:
            return std::make_unique<Boolean_response_widget>();
        case Eval_item::Type::Scale:
            return std::make_unique<Scale_response_widget>();
        case Eval_item::Type::Informational:
            return std::make_unique<Informational_response_widget>();
    }
}

void Response_widget::signal_change()
{
    changed().emit();
}

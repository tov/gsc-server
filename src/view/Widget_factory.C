#include "Widget_factory.h"

#include <Wt/WButtonGroup>
#include <Wt/WContainerWidget>
#include <Wt/WRadioButton>
#include <Wt/WSlider>
#include <Wt/WText>
#include <Wt/WTextArea>

Abstract_widget_base::Abstract_widget_base(Wt::WContainerWidget* parent)
        : WCompositeWidget(parent)
        , container_(new Wt::WContainerWidget)
{
    setImplementation(container_);
}

class Editable_explanation_holder : public Abstract_explanation_holder
{
public:
    Editable_explanation_holder(Wt::WContainerWidget* parent = nullptr);

    std::string explanation() const override;
    void set_explanation(const std::string&) override;

private:
    Wt::WTextArea* explanation_;
};

Editable_explanation_holder::Editable_explanation_holder(
        Wt::WContainerWidget* parent) : Abstract_explanation_holder(parent)
{
    auto container = new Wt::WContainerWidget();

    new Wt::WText(
            "<p>Explain, including line references (e.g. L14):</p>",
            container);

    explanation_ = new Wt::WTextArea(container);
    explanation_->setStyleClass("explanation");
    explanation_->setInline(false);

    setImplementation(container);
}

std::string Editable_explanation_holder::explanation() const
{
    return explanation_->text().toUTF8();
}

void Editable_explanation_holder::set_explanation(const std::string& text)
{
    explanation_->setText(Wt::WString::fromUTF8(text));
}

class Viewable_explanation_holder : public Abstract_explanation_holder
{
public:
    Viewable_explanation_holder(Wt::WContainerWidget* parent = nullptr);

    std::string explanation() const override;
    void set_explanation(const std::string&) override;

private:
    Wt::WText* explanation_;
};

Viewable_explanation_holder::Viewable_explanation_holder(
        Wt::WContainerWidget* parent) : Abstract_explanation_holder(parent)
{
    auto container = new Wt::WContainerWidget();
    explanation_ = new Wt::WText(container);
    setImplementation(container);
}

std::string Viewable_explanation_holder::explanation() const
{
    return explanation_->text().toUTF8();
}

void Viewable_explanation_holder::set_explanation(const std::string& text)
{
    explanation_->setText(Wt::WString::fromUTF8(text));
}

class Editable_boolean_option : public Abstract_boolean_option
{
public:
    Editable_boolean_option(Wt::WContainerWidget* parent = nullptr);

    bool value() const override;
    void set_value(bool) override;
    void clear() override;

private:
    Wt::WButtonGroup* no_yes_;
    Wt::WRadioButton* no_;
    Wt::WRadioButton* yes_;
};

Editable_boolean_option::Editable_boolean_option(Wt::WContainerWidget* parent)
        : Abstract_boolean_option(parent)
{
    no_yes_ = new Wt::WButtonGroup(container_);
    no_yes_->addButton(no_ = new Wt::WRadioButton("No", container_));
    no_yes_->addButton(yes_ = new Wt::WRadioButton("Yes", container_));

    no_yes_->checkedChanged().connect(std::bind([=]() { changed().emit(); }));
}

bool Editable_boolean_option::value() const
{
    return no_yes_->checkedButton() == yes_;
}

void Editable_boolean_option::set_value(bool value)
{
    no_yes_->setCheckedButton(value ? yes_ : no_);
}

void Editable_boolean_option::clear()
{
    no_yes_->setCheckedButton(nullptr);
}

class Viewable_boolean_option : public Abstract_boolean_option
{
public:
    Viewable_boolean_option(Wt::WContainerWidget* parent = nullptr);

    bool value() const override;
    void set_value(bool) override;
    void clear() override;

private:
    Wt::WText* text_;
};

Viewable_boolean_option::Viewable_boolean_option(Wt::WContainerWidget* parent)
        : Abstract_boolean_option(parent)
{
    text_ = new Wt::WText(container_);
}

bool Viewable_boolean_option::value() const
{
    return text_->text() == "Yes";
}

void Viewable_boolean_option::set_value(bool value)
{
    text_->setText(value ? "Yes" : "No");
}

void Viewable_boolean_option::clear()
{
    text_->setText("[not set]");
}

class Editable_unit_scale : public Abstract_unit_scale
{
public:
    Editable_unit_scale(Wt::WContainerWidget* parent = nullptr);

    double value() const override;
    void set_value(double) override;
    void clear() override;

private:
    Wt::WSlider* slider_;
    Wt::WText* number_;

    void update_number_();
};

Editable_unit_scale::Editable_unit_scale(Wt::WContainerWidget* parent)
        : Abstract_unit_scale(parent)
{
    slider_ = new Wt::WSlider(container_);
    slider_->resize(200, 50);
    slider_->setTickPosition(Wt::WSlider::TicksAbove);
    slider_->setTickInterval(20);
    slider_->setMinimum(0);
    slider_->setMaximum(100);

//    new Wt::WBreak(score_holder_);

    number_ = new Wt::WText(container_);

    slider_->valueChanged().connect(this, &Editable_unit_scale::update_number_);
}

double Editable_unit_scale::value() const
{
    return slider_->value() / 100.0;
}

void Editable_unit_scale::set_value(double d)
{
    slider_->setValue(int(100 * d));
    update_number_();
}

void Editable_unit_scale::clear()
{
    slider_->setValue(50);
    number_->setText("");
}

void Editable_unit_scale::update_number_()
{
    number_->setText(slider_->valueText() + "%");
}

class Viewable_unit_scale : public Abstract_unit_scale
{
public:
    Viewable_unit_scale(Wt::WContainerWidget* parent = nullptr);

    double value() const override;
    void set_value(double) override;
    void clear() override;

private:
    double value_;
    Wt::WText* text_;
};

Viewable_unit_scale::Viewable_unit_scale(Wt::WContainerWidget* parent)
        : Abstract_unit_scale(parent)
{
    text_ = new Wt::WText(container_);
}

double Viewable_unit_scale::value() const
{
    return value_;
}

void Viewable_unit_scale::set_value(double d)
{
    value_ = d;
    text_->setText(boost::lexical_cast<std::string>(int(100 * d)) + "%");
}

void Viewable_unit_scale::clear()
{
    value_ = 0;
    text_->setText("[not set]");
}

struct Editable_widget_factory : Abstract_widget_factory
{
    virtual Abstract_explanation_holder*
    make_explanation_holder(Wt::WContainerWidget* parent) const override
    {
        return new Editable_explanation_holder(parent);
    }

    virtual Abstract_boolean_option*
    make_boolean_option(Wt::WContainerWidget* parent) const override
    {
        return new Editable_boolean_option(parent);
    }

    virtual Abstract_unit_scale*
    make_unit_scale(Wt::WContainerWidget* parent) const override
    {
        return new Editable_unit_scale(parent);
    }

    virtual bool is_editable() const override
    {
        return true;
    }
};

struct Viewable_widget_factory : Abstract_widget_factory
{
    virtual Abstract_explanation_holder*
    make_explanation_holder(Wt::WContainerWidget* parent) const override
    {
        return new Viewable_explanation_holder(parent);
    }

    virtual Abstract_boolean_option*
    make_boolean_option(Wt::WContainerWidget* parent) const override
    {
        return new Viewable_boolean_option(parent);
    }

    virtual Abstract_unit_scale*
    make_unit_scale(Wt::WContainerWidget* parent) const override
    {
        return new Viewable_unit_scale(parent);
    }

    virtual bool is_editable() const override
    {
        return false;
    }
};

widget_factory_ptr make_editable_widget_factory()
{
    return std::make_unique<Editable_widget_factory>();
}

widget_factory_ptr make_viewable_widget_factory()
{
    return std::make_unique<Viewable_widget_factory>();
}

#include "Widget_factory.h"

#include <Wt/WButtonGroup>
#include <Wt/WContainerWidget>
#include <Wt/WRadioButton>
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

class Viewable_boolean_option : public Abstract_boolean_option
{
public:
    Viewable_boolean_option(Wt::WContainerWidget* parent = nullptr);

    bool value() const override;
    void set_value(bool) override;

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

struct Editable_widget_factory : Abstract_widget_factory
{
    virtual Abstract_explanation_holder*
    explanation_holder(Wt::WContainerWidget* parent) const override
    {
        return new Editable_explanation_holder(parent);
    }

    virtual Abstract_boolean_option*
    boolean_option(Wt::WContainerWidget* parent) const override
    {
        return new Editable_boolean_option(parent);
    }

    virtual bool is_editable() const override
    {
        return true;
    }
};

struct Viewable_widget_factory : Abstract_widget_factory
{
    virtual Abstract_explanation_holder*
    explanation_holder(Wt::WContainerWidget* parent) const override
    {
        return new Viewable_explanation_holder(parent);
    }

    virtual Abstract_boolean_option*
    boolean_option(Wt::WContainerWidget* parent) const override
    {
        return new Viewable_boolean_option(parent);
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

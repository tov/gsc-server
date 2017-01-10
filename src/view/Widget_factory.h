#pragma once

#include <Wt/WCompositeWidget>

#include <memory>
#include <string>

class Abstract_widget_base : public Wt::WCompositeWidget
{
public:
    Abstract_widget_base(Wt::WContainerWidget* parent = nullptr);

    virtual ~Abstract_widget_base() { }

protected:
    Wt::WContainerWidget* const container_;
};

class Abstract_explanation_holder : public Abstract_widget_base
{
public:
    Abstract_explanation_holder(Wt::WContainerWidget* parent = nullptr)
            : Abstract_widget_base(parent) { }

    virtual std::string explanation() const = 0;
    virtual void set_explanation(const std::string&) = 0;
};

class Abstract_boolean_option : public Abstract_widget_base
{
public:
    Abstract_boolean_option(Wt::WContainerWidget* parent = nullptr)
            : Abstract_widget_base(parent) { }

    virtual bool value() const = 0;
    virtual void set_value(bool) = 0;
    virtual void clear() = 0;

    Wt::Signal<>& changed() { return changed_; }

private:
    Wt::Signal<> changed_;
};

class Abstract_unit_scale : public Abstract_widget_base
{
public:
    Abstract_unit_scale(Wt::WContainerWidget* parent = nullptr)
            : Abstract_widget_base(parent) { }

    virtual double value() const = 0;
    virtual void set_value(double) = 0;
    virtual void clear() = 0;

    Wt::Signal<>& changed() { return changed_; }

private:
    Wt::Signal<> changed_;
};

struct Abstract_widget_factory
{
    virtual Abstract_explanation_holder*
    explanation_holder(Wt::WContainerWidget* parent = nullptr) const = 0;

    virtual Abstract_boolean_option*
    boolean_option(Wt::WContainerWidget* parent = nullptr) const = 0;

    virtual Abstract_unit_scale*
    unit_scale(Wt::WContainerWidget* parent = nullptr) const = 0;

    virtual bool is_editable() const = 0;

    virtual ~Abstract_widget_factory()
    { }
};

using widget_factory_ptr = std::unique_ptr<Abstract_widget_factory>;

widget_factory_ptr make_editable_widget_factory();
widget_factory_ptr make_viewable_widget_factory();


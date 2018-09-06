#pragma once

#include "../model/Eval_item.h"

#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>

class Response_widget : public Wt::WCompositeWidget
{
public:
    Response_widget(Wt::WContainerWidget* parent = nullptr);

    static std::unique_ptr<Response_widget>
    create(Eval_item::Type, Wt::WContainerWidget* parent = nullptr);

    virtual bool is_ready() const = 0;

    virtual double value() const = 0;
    virtual void set_value(double) = 0;
    virtual std::string explanation() const = 0;
    virtual void set_explanation(const std::string&) = 0;

    Wt::Signal<>& changed() { return changed_; }

    virtual ~Response_widget() { }

protected:
    Wt::WContainerWidget* container_;
    void signal_change();

private:
    Wt::Signal<> changed_;
};

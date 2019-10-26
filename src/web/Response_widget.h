#pragma once

#include "../model/Eval_item.h"

#include <Wt/WCompositeWidget.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WSignal.h>

class Response_widget : public Wt::WCompositeWidget
{
public:
    static std::unique_ptr<Response_widget>
    create(Eval_item::Type);

    virtual bool is_ready() const = 0;

    virtual double value() const = 0;
    virtual void set_value(double) = 0;
    virtual std::string explanation() const = 0;
    virtual void set_explanation(const std::string&) = 0;

    Wt::Signal<>& changed() { return changed_; }

    virtual ~Response_widget() { }

protected:
    Response_widget();

    Wt::WContainerWidget* container_;
    void signal_change();

private:
    Wt::Signal<> changed_;
};

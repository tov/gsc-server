#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>
#include <Wt/WSignal.h>

class Confirmation_dialog : public Wt::WDialog
{
public:
    Confirmation_dialog(const std::string& message,
                        Wt::WObject* parent = nullptr);

    Wt::Signal<>& accepted() { return accepted_; }
    Wt::Signal<>& rejected() { return rejected_; }

private:
    Wt::Signal<> accepted_;
    Wt::Signal<> rejected_;

    void finished_(Wt::WDialog::DialogCode);
};
#pragma once

#include <Wt/WDialog.h>
#include <Wt/WSignal.h>

class Confirmation_dialog : public Wt::WDialog
{
public:
    static Confirmation_dialog& create(const std::string& message);

    Wt::Signal<>& accepted() { return accepted_; }
    Wt::Signal<>& rejected() { return rejected_; }

private:
    Wt::Signal<> accepted_;
    Wt::Signal<> rejected_;

    explicit Confirmation_dialog(const std::string&);

    void finished_(Wt::DialogCode);
};
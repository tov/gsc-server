#pragma once

#include <Wt/WMessageBox.h>
#include <Wt/WWidget.h>

#include <sstream>
#include <string>

class Notification
{
public:
    Notification(std::string const& title, Wt::WWidget* parent);

    template <class T>
    Notification& operator<<(T const& arg)
    {
        message_ << arg;
        return *this;
    }

    ~Notification();

    Notification(Notification const&) = delete;
    Notification(Notification&&) = delete;
    Notification& operator=(Notification const&) = delete;
    Notification& operator=(Notification&&) = delete;

private:
    std::string title_;
    std::ostringstream message_;
    Wt::WWidget* parent_;
};


#pragma once

#include <Wt/WMessageBox.h>
#include <Wt/WWidget.h>

#include <functional>
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

    template <class T>
    Notification& and_then(T callback)
    {
        callback_ = std::move(callback);
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
    std::function<void()> callback_;
};


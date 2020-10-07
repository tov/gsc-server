#pragma once

#include <Wt/WContainerWidget.h>
#include <Wt/WMessageBox.h>

#include <functional>
#include <sstream>
#include <string>

class Notification {
public:
  Notification(std::string const &title, Wt::Icon = Wt::Icon::Critical);

  template <class T> Notification &operator<<(T const &arg) {
    message_ << arg;
    return *this;
  }

  template <class T> Notification &and_then(T callback) {
    callback_ = std::move(callback);
    return *this;
  }

  operator std::ostream &() { return message_; }

  ~Notification();

  Notification(Notification const &) = delete;
  Notification(Notification &&) = delete;
  Notification &operator=(Notification const &) = delete;
  Notification &operator=(Notification &&) = delete;

private:
  std::string title_;
  std::ostringstream message_;
  Wt::Icon icon_;
  std::function<void()> callback_;
};

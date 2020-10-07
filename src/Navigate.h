#pragma once

#include <Wt/WApplication.h>

class Navigate {
public:
  explicit Navigate(std::string uri, bool external = false)
      : uri_(std::move(uri)), external_(external) {}

  static void to(const std::string &uri, bool external = false) {
    if (external) {
      Wt::WApplication::instance()->redirect(uri);
    } else {
      Wt::WApplication::instance()->setInternalPath(uri, true);
    }
  }

  template <typename... T> void operator()(T &&...) { to(uri_, external_); }

private:
  std::string uri_;
  bool external_;
};

template <> struct Wt::Signals::Impl::function_traits<Navigate> {
  static const size_t argCount = 0;
  using function = std::function<void()>;
};

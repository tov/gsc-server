#pragma once

#include "../../common/env_var.h"

#include <optional>
#include <string>

#include <Wt/Http/Request.h>
#include <Wt/WEnvironment.h>

struct Environment {
  virtual bool invalid() const = 0;

  explicit operator bool() const { return !invalid(); }

  virtual std::string const *get_parameter(std::string const &) const = 0;

  virtual std::string header_value(std::string const &) const = 0;

  virtual ~Environment() = default;
};

class App_environment : public Environment {
public:
  App_environment();

  bool invalid() const override;
  std::string const *get_parameter(std::string const &string) const override;
  std::string header_value(std::string const &string) const override;

private:
  Wt::WEnvironment const *environment_;
};

class Req_environment : public Environment {
public:
  explicit Req_environment(Wt::Http::Request const &);

  bool invalid() const override;
  std::string const *get_parameter(std::string const &) const override;
  std::string header_value(std::string const &) const override;

private:
  Wt::Http::Request const &request_;
};

// Attempts to acquire an authenticated remote user
// from the given environment.
std::optional<std::string>
env_remote_user(Environment const & = App_environment());

// Gets the contents of the `whoami` parameter set by
// `redirect_with_whoami`.
std::optional<std::string>
param_whoami(Environment const & = App_environment());

// Redirects to here + "?whoami={username}"
void redirect_with_whoami(std::string const &username);

// Redirects to "{uri}?whoami={username}"
void redirect_with_whoami(std::string const &uri, std::string const &username);

///
/// IMPLEMENTATIONS
///

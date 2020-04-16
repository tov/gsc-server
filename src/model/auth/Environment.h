#pragma once

#include <optional>
#include <string>

// Attempts to acquire an authenticated remote user
// from the environment.
std::optional<std::string> env_remote_user();

// Redirects to here + "?whoami={username}"
void redirect_with_whoami(std::string const& username);

// Redirects to "{uri}?whoami={username}"
void redirect_with_whoami(std::string const& uri,
                          std::string const& username);

// Gets the contents of the `whoami` parameter set by
// `redirect_with_whoami`.
std::optional<std::string> param_whoami();


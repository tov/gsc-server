#include "env_var.h"

#include <cstdlib>

const char *get_env_var(const char *name, const char *def) {
  auto value = std::getenv(name);
  return value ? value : def;
}

const char *get_env_var(std::string const &name, const char *def) {
  return get_env_var(name.c_str(), def);
}

const char *get_env_var(std::string_view name, const char *def) {
  return get_env_var(std::string(name), def);
}

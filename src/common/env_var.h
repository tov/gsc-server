#pragma once

#include <string>
#include <string_view>

const char* get_env_var(const char*, const char* = "");
const char* get_env_var(std::string const&, const char* = "");
const char* get_env_var(std::string_view, const char* = "");

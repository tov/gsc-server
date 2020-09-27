#include "strarray.h"
#include <cstdlib>
#include <cstring>

strarray::strarray() {
  g_strarray_.count = 0;
  g_strarray_.strings = nullptr;
}

strarray::strarray(const std::vector<std::string> &strings) {
  auto size = strings.size();
  g_strarray_.count = size;
  g_strarray_.strings = (char **)malloc(size * sizeof(char *));
  for (size_t i = 0; i < size; ++i) {
    auto length = strings[i].size() + 1;
    g_strarray_.strings[i] = (char *)malloc(length * sizeof(char));
    strncpy(g_strarray_.strings[i], strings[i].c_str(), length);
    g_strarray_.strings[i][length] = '\0';
  }
}

strarray::strarray(const git_strarray *g_strarray) {
  g_strarray_.count = g_strarray->count;
  g_strarray_.strings = (char **)malloc(g_strarray->count * sizeof(char *));
  for (size_t i = 0; i < g_strarray->count; ++i) {
    auto length = strlen(g_strarray->strings[i]) + 1;
    g_strarray_.strings[i] = (char *)malloc(length * sizeof(char));
    strncpy(g_strarray_.strings[i], g_strarray->strings[i], length);
    g_strarray_.strings[i][length] = '\0';
  }
}

strarray::~strarray() {
  if (g_strarray_.count)
    git_strarray_free(&g_strarray_);
}

strarray strarray::copy() const {
  strarray result;
  if (git_strarray_copy(&result.g_strarray_, &g_strarray_))
    throw git_exception();
  return result;
}

std::vector<std::string> strarray::to_vector() const {
  std::vector<std::string> result{};
  for (auto tag : *this)
    result.push_back(tag);
  return result;
}

const git_strarray *strarray::g_strarray() const { return &g_strarray_; }


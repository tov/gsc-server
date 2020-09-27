#include "oid.h"
#include <iostream>

oid::oid() {}

oid::oid(const std::string &hex_string) {
  if (git_oid_fromstr(&g_oid_, hex_string.c_str()))
    throw git_exception();
}

oid::oid(const std::string &hex_string, size_t length) {
  if (git_oid_fromstrn(&g_oid_, hex_string.c_str(), length))
    throw git_exception();
}

oid::oid(const git_oid *g_oid) {
  // Convert git_oid * to string and construct this oid
  size_t n = GIT_OID_HEXSZ;
  std::string buffer(n, '0');
  if (!git_oid_tostr(const_cast<char *>(buffer.c_str()), n + 1, g_oid))
    throw git_exception();
  // Construct from string
  if (git_oid_fromstr(&g_oid_, buffer.c_str()))
    throw git_exception();
}

oid::oid(const unsigned char *raw) { git_oid_fromraw(&g_oid_, raw); }

int oid::compare(const oid &rhs) const {
  return git_oid_cmp(&g_oid_, rhs.g_oid());
}

int oid::compare(const oid &rhs, size_t length) const {
  return git_oid_ncmp(&g_oid_, rhs.g_oid(), length);
}

oid oid::copy() const {
  oid result;
  git_oid_cpy(&result.g_oid_, &g_oid_);
  return result;
}

bool oid::is_zero() const { return git_oid_iszero(&g_oid_); }

bool oid::operator==(const oid &rhs) const {
  return git_oid_equal(&g_oid_, rhs.g_oid());
}

bool oid::operator==(const std::string &rhs) const {
  return git_oid_streq(&g_oid_, rhs.c_str()) ? false : true;
}

std::string oid::to_hex_string(size_t n) const {
  std::string result(n, '0');
  if (!git_oid_tostr(const_cast<char *>(result.c_str()), n + 1, &g_oid_))
    throw git_exception();
  return result;
}

std::string oid::to_path_string() const {
  std::string result(GIT_OID_HEXSZ + 1, '0');
  git_oid_pathfmt(const_cast<char *>(result.c_str()), &g_oid_);
  return result;
}

git_oid *oid::g_oid() { return &g_oid_; }

const git_oid *oid::g_oid() const { return &g_oid_; }

#include "Media_type_registry.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <ios>
#include <regex>
#include <sstream>

namespace {

std::regex const registry_entry_regex("([^=]+)=(.+)");
std::regex const file_extension_regex(".*\\.([^.]*)");

std::string const default_type = "application/octet-stream";

struct Media_type {
  std::string name;
  std::string extension;
};

struct ToLower {
  char operator()(char c) { return static_cast<char>(std::tolower(c)); }
};

template <typename InputIter>
void lcase_assign(std::string &buffer, InputIter source, InputIter limit) {
  buffer.clear();
  buffer.reserve(limit - source);
  std::transform(source, limit, back_inserter(buffer), ToLower{});
}

std::istream &operator>>(std::istream &is, Media_type &out) {
  std::string buffer;
  std::smatch sm;

  if (is >> buffer && std::regex_match(buffer, sm, registry_entry_regex)) {
    lcase_assign(out.extension, sm[1].first, sm[1].second);
    out.name = std::string(sm[2].first, sm[2].second);
  } else {
    is.setstate(is.failbit);
  }

  return is;
}

bool is_not_eof(std::istream &is) {
  is >> std::ws;
  return !is.eof();
}

static std::string failure_message(std::exception const &reason) {
  return "Media_type_registry: could not load media types"
         ";\n    reason: " +
         std::string(reason.what());
}

} // namespace

struct Media_type_load_failure : std::runtime_error {
  explicit Media_type_load_failure(std::exception const &reason)
      : runtime_error(failure_message(reason)) {}
};

Media_type_registry::Media_type_registry() {
  try {
    load("media_types.dat");
  } catch (std::ios_base::failure const &exn) {
    throw Media_type_load_failure(exn);
  }
}

Media_type_registry const &Media_type_registry::instance() {
  static Media_type_registry instance;
  return instance;
}

void Media_type_registry::load(std::string const &filename) {
  std::ifstream file;
  file.exceptions(file.badbit | file.failbit);
  file.open(filename);

  Media_type media_type;

  while (is_not_eof(file) && file >> media_type) {
    media_types_[media_type.extension] = media_type.name;
  }
}

std::string const &
Media_type_registry::lookup(std::string const &filename) const {
  std::smatch sm;

  if (!std::regex_match(filename, sm, file_extension_regex))
    return default_type;

  std::string extension;
  lcase_assign(extension, sm[1].first, sm[1].second);

  if (auto iter = media_types_.find(extension); iter != media_types_.end())
    return iter->second;

  return default_type;
}

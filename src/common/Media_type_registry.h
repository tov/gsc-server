#pragma once

#include <string>
#include <unordered_map>

class Media_type_registry {
public:
  static Media_type_registry const &instance();

  std::string const &lookup(std::string const &filename) const;

private:
  Media_type_registry();

  void load(std::string const &filename);

  std::unordered_map<std::string, std::string> media_types_;
};

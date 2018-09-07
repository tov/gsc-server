#pragma once

#include <string>
#include <unordered_map>

class Media_type_registry
{
public:
    void load(std::string const& filename);

    std::string const* lookup(std::string const& filename) const;

private:
    std::unordered_map<std::string, std::string> media_types_;
};

#include "Media_type.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <ios>
#include <regex>
#include <sstream>

static std::regex const registry_entry_regex("([^=]+)=(.+)");
static std::regex const file_extension_regex("\\.([^.]*)");

struct Media_type
{
    std::string name;
    std::string extension;
};

template <typename InputIter>
void lcase_assign(std::string& buffer, InputIter source, InputIter limit)
{
    buffer.clear();
    buffer.reserve(limit - source);

    transform(source, limit,
              back_inserter(buffer),
              [](char c) { return (int) tolower(c); });
}

std::istream& operator>>(std::istream& is, Media_type& media_type)
{
    std::string buffer;
    is >> buffer;
    if (!is) return is;

    std::smatch sm;
    if (! std::regex_match(buffer, sm, registry_entry_regex)) {
        is.setstate(std::ios::failbit);
        return is;
    }

    lcase_assign(media_type.extension, sm[1].first, sm[1].second);
    media_type.name = std::string(sm[2].first, sm[2].second);

    return is;
}

std::string const*
Media_type_registry::lookup(std::string const& filename) const
{
    std::smatch sm;

    if (!std::regex_match(filename, sm, file_extension_regex))
        return nullptr;

    std::string extension;
    lcase_assign(extension, sm[1].first, sm[1].second);

    auto iter = media_types_.find(extension);
    if (iter == media_types_.end())
        return nullptr;

    return &iter->second;
}

void Media_type_registry::load(std::string const& filename)
{
    std::ifstream file(filename);
    Media_type media_type;

    while (file >> media_type) {
        media_types_[media_type.extension] = media_type.name;
    }

    if (file.eof()) return;

    throw std::runtime_error{"Could not load media type registry from file: "
                             + filename};
}


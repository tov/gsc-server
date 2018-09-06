#pragma once

#include "Path.h"

#include <memory>
#include <string>

namespace api {

namespace Resource {

class Base
{
public:
    static std::unique_ptr<Base> parse(std::string const& path_info);

    virtual ~Base() = default;

private:
};

} // end namespace Resource

} // end namespace api

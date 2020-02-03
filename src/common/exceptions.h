#pragma once

#include <stdexcept>

struct Access_check_failed : public std::runtime_error
{
    using runtime_error::runtime_error;
};

struct Resource_not_found : public std::runtime_error
{
    Resource_not_found()
            : runtime_error("resource not found") { }
};

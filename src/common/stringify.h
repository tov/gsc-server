#pragma once

#include <array>
#include <string>
#include <type_traits>
#include <vector>

template <class T>
class Enum;

template <class T>
char const* stringify(T value)
{
    return Enum<T>::show(value);
}

template <class T>
T destringify(char const* value)
{
    return Enum<T>::read(value);
}

template <class T>
T destringify(std::string const& value)
{
    return Enum<T>::read(value.c_str());
}

template <class T>
struct Enumerator_info
{
    using enum_t = T;
    using repr_t = std::underlying_type_t<enum_t>;

    enum_t value;
    repr_t repr;
    char const* name;

    Enumerator_info(enum_t value, repr_t repr, char const* name) noexcept
            : value{value}, repr{repr}, name{name}
    { }

    Enumerator_info(enum_t value, char const* name) noexcept
            : Enumerator_info{value, static_cast<repr_t>(value), name}
    { }
};

template <class T, size_t n>
using Enum_info = std::array<Enumerator_info<T>, n>;


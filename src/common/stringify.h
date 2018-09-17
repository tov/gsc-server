#pragma once

#include <string>


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


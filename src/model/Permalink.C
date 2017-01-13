#include "Permalink.h"

#include <random>

namespace {

std::default_random_engine generator;
std::uniform_int_distribution<int> distribution(0, 35);

char random_char()
{
    int value = distribution(generator);

    if (value < 10)
        return value + '0';
    else
        return value - 10 + 'a';
}

}

std::string create_permalink(size_t length)
{
    std::string result(length, '\0');

    for (char& c : result)
        c = random_char();

    return result;
}

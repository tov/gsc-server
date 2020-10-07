#include "Permalink.h"

#include <random>

namespace {

class Char_generator {
public:
  Char_generator();
  char operator()();

private:
  std::mt19937_64 generator_;
  std::uniform_int_distribution<int> distribution_;
};

Char_generator::Char_generator() : distribution_(0, 35) {
  generator_.seed(std::random_device{}());
}

char Char_generator::operator()() {
  int value = distribution_(generator_);

  if (value < 10)
    return value + '0';
  else
    return value - 10 + 'a';
}

thread_local Char_generator random_char;

} // namespace

std::string create_permalink(size_t length) {
  std::string result(length, '\0');

  for (char &c : result)
    c = random_char();

  return result;
}

#include "Api_key.h"

#include <Wt/Auth/User.h>

#include <chrono>
#include <random>

const std::string api_key_identity = "apikey";
const std::string api_key_name = "gsc_api_key";

namespace {

using Engine = std::mt19937_64;
using Distro = std::uniform_int_distribution<int>;

Engine construct_engine() {
  std::random_device rd;
  auto time = static_cast<std::random_device::result_type>(
      std::chrono::high_resolution_clock().now().time_since_epoch().count());
  return Engine(rd() ^ time);
}

const size_t api_key_size = 40;
const Distro digit_distribution(0, 15);

char random_digit(Distro &distro, Engine &engine) {
  auto i = distro(engine);

  if (i < 10)
    return '0' + i;
  else if (i < 36)
    return 'a' + (i - 10);
  else
    return '_';
}

std::string generate_api_key() {
  auto engine = construct_engine();
  auto distro = digit_distribution;
  auto result = std::string(api_key_size, ' ');

  for (char &digit : result) {
    digit = random_digit(distro, engine);
  }

  return result;
}

} // end anonymous namespace

void set_api_key_identity(Wt::Auth::User &auth_user) {
  auth_user.setIdentity(api_key_identity, generate_api_key());
}

std::string get_api_key_identity(Wt::Auth::User const &auth_user) {
  return auth_user.identity(api_key_identity).toUTF8();
}

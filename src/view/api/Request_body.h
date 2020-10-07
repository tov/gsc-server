#pragma once

#include "../../common/util.h"

#include <Wt/Http/Request.h>
#include <Wt/Json/Value.h>

#include <functional>
#include <istream>
#include <string>

class Bytes;

namespace api {

class Request_body {
public:
  explicit Request_body(Wt::Http::Request const &request)
      : in_{request.in()}, size_{request.contentLength()} {}

  int size() const { return size_; };

  std::string read_string() &&;
  Bytes read_bytes() &&;
  Wt::Json::Value read_json() &&;

private:
  std::reference_wrapper<std::istream> in_;
  int size_;
};

} // end namespace api

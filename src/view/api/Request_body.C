#include "../../common/util.h"
#include "../../model/File_data.h"
#include "../Http_status.h"
#include "Request_body.h"

#include <Wt/Json/Parser.h>

namespace api {

J::Value Request_body::read_json() && {
  string buffer{move(*this).read_string()};
  J::ParseError error;
  J::Value result;

  if (J::parse(buffer, result, error))
    return result;

  throw Http_status{400, "Request body expected to be JSON"};
}

Bytes Request_body::read_bytes() && {
  Bytes result{in_, size_};
  size_ = 0;
  return result;
}

string Request_body::read_string() && {
  string result;
  result.reserve(size_);
  result.assign(istreambuf_iterator<char>(in_), {});

  size_ = 0;

  return result;
}

} // end namespace api

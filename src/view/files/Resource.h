#pragma once

#include "../Resource_base.h"

namespace files {

class Resource : public Resource_base {
public:
  using Resource_base::Resource_base;

protected:
  void handleRequest(Wt::Http::Request const &, Response &, Session &) override;
};

} // end namespace files

#pragma once

#include <functional>

namespace Wt {
namespace Dbo {}
namespace Json {}
} // namespace Wt

namespace dbo = Wt::Dbo;
namespace J = Wt::Json;

using namespace Wt;
using namespace std;

template <class T> using ref_wrap = reference_wrapper<T>;

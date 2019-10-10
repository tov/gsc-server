#pragma once

#include <functional>

namespace Wt {
namespace Dbo { }
namespace Json { }
}

namespace dbo = Wt::Dbo;
namespace J = Wt::Json;

using namespace std;

template <class T>
using ref_wrap = reference_wrapper<T>;

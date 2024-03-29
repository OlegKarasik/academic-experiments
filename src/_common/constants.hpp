#pragma once

#include <limits>

namespace utilz {
namespace constants {

template<typename T>
constexpr T
infinity()
{
  return ((std::numeric_limits<T>::max)() / T(2)) - T(1);
};

} // namespace constants
} // namespace utilz

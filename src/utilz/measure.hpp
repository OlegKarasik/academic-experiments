#include <chrono>

namespace utilz {

template<typename TDuration, typename TFunction>
TDuration
measure(TFunction fn)
{
  auto start = std::chrono::high_resolution_clock::now();

  fn();

  auto stop = std::chrono::high_resolution_clock::now();

  return std::chrono::duration_cast<TDuration>(stop - start);
};

template<typename TFunction>
int64_t
measure_milliseconds(TFunction fn)
{
  return measure<std::chrono::milliseconds>(fn).count();
};

} // namespace utilz

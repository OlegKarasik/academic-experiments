#pragma once

#include <chrono>
#include <string>
#include <map>

#ifdef ENABLE_SCOPE_MEASUREMENTS
  #define SCOPE_MEASURE_MILLISECONDS(KEY) utilz::auto_measurement<std::chrono::milliseconds> __auto__measurement(KEY)
#else
  #define SCOPE_MEASURE_MILLISECONDS(KEY)
#endif

namespace utilz {

// ---
// Forward declarations
//

template<typename TDuration>
class auto_measurement;

//
// Forward declarations
// ---

template<typename TDuration>
static std::unordered_map<std::string, std::vector<TDuration>> auto_measurements;

template<typename TDuration, typename TFunction, typename... Args>
TDuration
measure(TFunction fn, Args&&... args)
{
  auto start = std::chrono::high_resolution_clock::now();

  fn(std::forward<Args>(args)...);

  auto stop = std::chrono::high_resolution_clock::now();

  return std::chrono::duration_cast<TDuration>(stop - start);
};

template<typename TFunction, typename... Args>
int64_t
measure_milliseconds(TFunction fn, Args&&... args)
{
  return measure<std::chrono::milliseconds>(fn, std::forward<Args>(args)...).count();
};

template<typename TDuration>
class auto_measurement
{
  private:
    std::string m_key;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;

  public:
    auto_measurement(std::string key)
      : m_key(key)
    {
      this->m_start = std::chrono::high_resolution_clock::now();
    }

    ~auto_measurement()
    {
      auto stop = std::chrono::high_resolution_clock::now();

      auto it = auto_measurements<TDuration>.find(this->m_key);
      if (it == auto_measurements<TDuration>.end()) {
        auto_measurements<TDuration>.emplace(this->m_key, std::vector<TDuration> { std::chrono::duration_cast<TDuration>(stop - this->m_start) });
      } else {
        auto_measurements<TDuration>[this->m_key].push_back(std::chrono::duration_cast<TDuration>(stop - this->m_start));
      }
    }
};

} // namespace utilz

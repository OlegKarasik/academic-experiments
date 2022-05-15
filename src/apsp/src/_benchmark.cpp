#include <filesystem>
#include <fstream>

#include "benchmark/benchmark.h"

// internals
#include "workspace.hpp"

// global utilz
#include "square-shape.hpp"

// local includes
//
#include "algorithm.hpp"
#include "io.hpp"

namespace apsp  = ::apsp;
namespace utilz = ::utilz;

template<typename T>
class Fixture : public benchmark::Fixture
{
// aliasing
//
#ifdef APSP_ALG_WIND_UPDOWN
  using buffer = ::utilz::memory::buffer_dyn;
#endif

#ifdef APSP_ALG_BLOCKED
  using matrix = ::utilz::square_shape<::utilz::square_shape<T>>;
#else
  using matrix = ::utilz::square_shape<T>;
#endif

public:
#ifdef APSP_ALG_WIND_UPDOWN
  buffer m_buf;
#endif

  matrix m_src;

  Fixture()
  {
    std::filesystem::path root_path = workspace::root();
    std::filesystem::path src_path  = root_path / "data/_test/direct-acyclic-graphs/10-14.source.g";

    std::ifstream src_fs(src_path);
    if (!src_fs.is_open())
      throw std::logic_error("erro: the file '" + src_path.generic_string() + "' doesn't exist.");

#ifdef APSP_ALG_BLOCKED
    ::apsp::io::scan_matrix(src_fs, false, this->m_src, 2);
#else
    ::apsp::io::scan_matrix(src_fs, false, this->m_src);
#endif
  }
  ~Fixture()
  {
  }
};

BENCHMARK_TEMPLATE_DEFINE_F(Fixture, ExecuteInt, int)
(benchmark::State& state)
{
  for (auto _ : state) {
#ifdef APSP_ALG_WIND_UPDOWN
    auto options = wind_up_apsp(this->m_src, this->m_buf);

    calculate_apsp(this->m_src, options);

    wind_down_apsp(this->m_src, this->m_buf, options);
#else
    calculate_apsp(this->m_src);
#endif
  }
}

BENCHMARK_REGISTER_F(Fixture, ExecuteInt)->UseRealTime();
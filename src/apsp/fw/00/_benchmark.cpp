#include <filesystem>
#include <fstream>

#include "benchmark/benchmark.h"

// internals
#include "_internal.hpp"

// global utilz
#include "square-shape.hpp"

// local utilz
#include "../utilz/io.hpp"

// algorithm
#include "algorithms.hpp"

#ifdef _WIN32
#pragma comment(lib, "Shlwapi.lib")
#endif

namespace lutilz = ::fw::utilz;
namespace gutilz = ::utilz;

template<typename T>
class Fixture : public benchmark::Fixture
{
  using matrix    = utilz::square_shape<int>;
  using matrix_st = typename utilz::traits::square_shape_traits<matrix>::size_type;

public:
  matrix m_src;

  Fixture()
  {
    std::filesystem::path root_path = _internal::root();
    std::filesystem::path src_path  = root_path / "data/_test/direct-acyclic-graphs/10-14.source.g";

    std::ifstream src_fs(src_path);
    if (!src_fs.is_open())
      throw std::logic_error("erro: the file '" + src_path.generic_string() + "' doesn't exist.");

    lutilz::io::scan_matrix(src_fs, this->m_src);
  }
  ~Fixture()
  {
  }
};

BENCHMARK_TEMPLATE_DEFINE_F(Fixture, ExecuteInt, int)
(benchmark::State& state)
{
  for (auto _ : state)
    calculate_apsp(this->m_src);
}

BENCHMARK_REGISTER_F(Fixture, ExecuteInt)->UseRealTime();

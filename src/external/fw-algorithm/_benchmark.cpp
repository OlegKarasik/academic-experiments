#include "program.h"

#include "benchmark/benchmark.h"

#ifdef _WIN32
#pragma comment(lib, "Shlwapi.lib")
#endif

using namespace utilz;

// Constant value which indicates that there is no path between two vertexes.
// Please note: this value can be used ONLY when input paths are >= 0.
//
constexpr long
no_path_value()
{
  return ((std::numeric_limits<long>::max)() / 2) - 1;
};

class Fixture : public benchmark::Fixture
{
public:
  void SetUp(const ::benchmark::State& state)
  {
    long* mem = (long*)malloc(7 * 7 * sizeof(long));
    long  sz  = 7;

    std::fill_n(mem, sz * sz, no_path_value());

    rect_shape<long> shape(mem, sz, sz);
    shape(1, 2) = 9;
    shape(1, 3) = 2;
    shape(1, 6) = 5;
    shape(3, 4) = 3;
    shape(3, 6) = 6;
    shape(4, 2) = 1;
    shape(4, 6) = 4;

    this->shape = shape;
  }

  void TearDown(const ::benchmark::State& state)
  {
    //free(this->m_value.first);
  }

  rect_shape<long> shape;
};

BENCHMARK_F(Fixture, Base)
(benchmark::State& st)
{
  for (auto _ : st)
    base_impl(shape);
}

BENCHMARK_F(Fixture, O1)
(benchmark::State& st)
{
  for (auto _ : st)
    o1_impl(shape);
}

BENCHMARK_F(Fixture, O2)
(benchmark::State& st)
{
  for (auto _ : st)
    o2_impl(shape);
}

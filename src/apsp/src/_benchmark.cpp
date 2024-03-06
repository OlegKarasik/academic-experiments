// benchmarks
//
#include "benchmark/benchmark.h"

// global includes
//
#include <array>
#include <filesystem>
#include <fstream>

// local internals
//
#include "workspace.hpp"

// local utilz
#include "graphs-io.hpp"
#include "matrix-io.hpp"
#include "matrix-manip.hpp"
#include "matrix-traits.hpp"
#include "matrix.hpp"

// local includes
//
#include "algorithm.hpp"

#if defined(APSP_ALG_HAS_BLOCKS)
const auto parameters = std::array<std::tuple<std::string, size_t>, 6>(
  { std::make_tuple("10-14.source.g", 2),
    std::make_tuple("10-14.source.g", 4),
    std::make_tuple("10-14.source.g", 5),
    std::make_tuple("32-376.source.g", 2),
    std::make_tuple("32-376.source.g", 4),
    std::make_tuple("32-376.source.g", 5) });
#elif defined(APSP_ALG_HAS_UNEQUAL_BLOCKS)
const auto parameters = std::array<std::tuple<std::string, std::vector<size_t>>, 2>(
  { std::make_tuple("10-14.source.g", std::vector<size_t>{ 2, 3, 3, 2 }),
    std::make_tuple("32-376.source.g", std::vector<size_t>{ 4, 5, 10, 4, 4, 5 }) });
#else
const auto parameters = std::array<std::tuple<std::string>, 2>({ "10-14.source.g", "32-376.source.g" });
#endif

template<typename T>
class Fixture : public benchmark::Fixture
{
// aliasing
//
#ifdef APSP_ALG_HAS_OPTIONS
  using buffer = utilz::memory::buffer_dyn;
#endif

#if defined(APSP_ALG_HAS_BLOCKS)
  using matrix = utilz::square_matrix<::utilz::square_matrix<T>>;
#elif defined(APSP_ALG_HAS_UNEQUAL_BLOCKS)
  using matrix = utilz::square_matrix<::utilz::rect_matrix<T>>;
#else
  using matrix = utilz::square_matrix<T>;
#endif

public:
#ifdef APSP_ALG_HAS_OPTIONS
  buffer m_buf;
#endif

  std::vector<matrix> m_src;

  Fixture()
  {
    utilz::graphs::io::graph_format format = utilz::graphs::io::graph_format::graph_fmt_weightlist;

    std::filesystem::path root_path = workspace::root();

    for (auto params : parameters) {
      std::filesystem::path src_path = root_path / "data/_test/direct-acyclic-graphs/" / std::get<0>(params);

      std::ifstream src_fs(src_path);
      if (!src_fs.is_open())
        throw std::logic_error("erro: the file '" + src_path.generic_string() + "' doesn't exist.");

      matrix src_matrix;

#if defined(APSP_ALG_HAS_BLOCKS)
      utilz::graphs::io::scan_graph(format, src_fs, src_matrix, std::get<1>(params));
#elif defined(APSP_ALG_HAS_UNEQUAL_BLOCKS)
      utilz::graphs::io::scan_graph(format, src_fs, src_matrix, std::get<1>(params));
#else
      utilz::graphs::io::scan_graph(format, src_fs, src_matrix);
#endif

      this->m_src.push_back(std::move(src_matrix));
    }
  }
  ~Fixture()
  {
  }
};

BENCHMARK_TEMPLATE_DEFINE_F(Fixture, ExecuteInt, int)
(benchmark::State& state)
{
  for (auto _ : state) {
    auto src_index = state.range(0);

#ifdef APSP_ALG_HAS_OPTIONS
    auto options = up(this->m_src[src_index], this->m_buf);
#endif

    auto start = std::chrono::high_resolution_clock::now();

#ifdef APSP_ALG_HAS_OPTIONS
    run(this->m_src[src_index], options);
#else
    run(this->m_src[src_index]);
#endif

    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_seconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(
        end - start);

    state.SetIterationTime(elapsed_seconds.count());

#ifdef APSP_ALG_HAS_OPTIONS
    down(this->m_src[src_index], this->m_buf, options);
#endif
  }
}

BENCHMARK_REGISTER_F(Fixture, ExecuteInt)
  ->DenseRange(0, parameters.size() - 1, 1)
  ->UseManualTime()
  ->DisplayAggregatesOnly()
  ->Repetitions(10);

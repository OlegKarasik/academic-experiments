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
#include "square-shape.hpp"
#include "shapes-traits.hpp"
#include "shapes-manip.hpp"
#include "shapes-io.hpp"

// local includes
//
#include "algorithm.hpp"

const auto graph_names = std::array<std::string, 2>({ "10-14.source.g", "32-376.source.g" });

#ifdef APSP_ALG_HAS_BLOCKS
const auto block_sizes = std::array<int, 3>({ 2, 4, 5 });
#endif

template<typename T>
class Fixture : public benchmark::Fixture
{
// aliasing
//
#ifdef APSP_ALG_HAS_OPTIONS
  using buffer = utilz::memory::buffer_dyn;
#endif

#ifdef APSP_ALG_HAS_BLOCKS
  using matrix = utilz::square_matrix<::utilz::square_matrix<T>>;
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

#ifdef APSP_ALG_HAS_BLOCKS
    for (auto block_size : block_sizes) {
#endif
      for (auto graph_name : graph_names) {
        std::filesystem::path src_path = root_path / "data/_test/direct-acyclic-graphs/" / graph_name;

        std::ifstream src_fs(src_path);
        if (!src_fs.is_open())
          throw std::logic_error("erro: the file '" + src_path.generic_string() + "' doesn't exist.");

        matrix src_matrix;

#ifdef APSP_ALG_HAS_BLOCKS
        utilz::graphs::io::scan_graph(format, src_fs, src_matrix, block_size);
#else
        utilz::graphs::io::scan_graph(format, src_fs, src_matrix);
#endif

        this->m_src.push_back(std::move(src_matrix));
      }
#ifdef APSP_ALG_HAS_BLOCKS
    }
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
#ifdef APSP_ALG_HAS_BLOCKS
  ->DenseRange(0, graph_names.size() * block_sizes.size() - 1, 1)
#else
  ->DenseRange(0, graph_names.size() - 1, 1)
#endif
  ->UseManualTime()
  ->DisplayAggregatesOnly()
  ->Repetitions(10);

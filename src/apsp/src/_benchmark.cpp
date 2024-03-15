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

#ifdef APSP_ALG_MATRIX_BLOCKS
const auto parameters = std::array<std::tuple<std::string, size_t>, 6>(
  { std::make_tuple("10-14.source.g", 2),
    std::make_tuple("10-14.source.g", 4),
    std::make_tuple("10-14.source.g", 5),
    std::make_tuple("32-376.source.g", 2),
    std::make_tuple("32-376.source.g", 4),
    std::make_tuple("32-376.source.g", 5) });
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
const auto parameters = std::array<std::tuple<std::string, std::string>, 2>(
  { std::make_tuple("10-14.source.g", "10-14.communities.g"),
    std::make_tuple("32-376.source.g", "32-376.communities.g") });
#endif

#ifdef APSP_ALG_MATRIX
const auto parameters = std::array<std::tuple<std::string>, 2>({ "10-14.source.g", "32-376.source.g" });
#endif

template<typename T>
class Fixture : public benchmark::Fixture
{
public:
// aliasing
//
#ifdef APSP_ALG_EXTRA_OPTIONS
  using buffer = utilz::memory::buffer_dyn;
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
  using matrix = utilz::square_matrix<::utilz::square_matrix<T>>;
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  using matrix           = utilz::square_matrix<::utilz::rect_matrix<T>>;
  using matrix_clusters  = utilz::matrix_clusters<typename utilz::square_matrix<::utilz::square_matrix<T>>::size_type>;
  using matrix_rearrange = utilz::procedures::matrix_rearrange<matrix>;
#endif

#ifdef APSP_ALG_MATRIX
  using matrix = utilz::square_matrix<T>;
#endif

public:
#ifdef APSP_ALG_EXTRA_OPTIONS
  buffer m_buf;
#endif

  std::vector<matrix> m_src;

#ifdef APSP_ALG_MATRIX_CLUSTERS
  std::vector<matrix_clusters> m_src_clusters;
#endif

  Fixture()
  {
    utilz::graphs::io::graph_format graph_format = utilz::graphs::io::graph_format::graph_fmt_weightlist;

#ifdef APSP_ALG_MATRIX_CLUSTERS
    utilz::communities::io::communities_format communities_format = utilz::communities::io::communities_format::communities_fmt_rlang;
#endif

    std::filesystem::path root_path = workspace::root();

    for (auto params : parameters) {
      std::filesystem::path src_graph_path = root_path / "data/_test/direct-acyclic-graphs/" / std::get<0>(params);

#ifdef APSP_ALG_MATRIX_CLUSTERS
      std::filesystem::path src_communities_path = root_path / "data/_test/direct-acyclic-graphs/" / std::get<1>(params);
#endif

      std::ifstream src_graph_fs(src_graph_path);
      if (!src_graph_fs.is_open())
        throw std::logic_error("erro: the file '" + src_graph_path.generic_string() + "' doesn't exist.");

#ifdef APSP_ALG_MATRIX_CLUSTERS
      std::ifstream src_communities_fs(src_communities_path);
      if (!src_communities_fs.is_open())
        throw std::logic_error("erro: the file '" + src_communities_path.generic_string() + "' doesn't exist.");
#endif

      matrix src_matrix;

#ifdef APSP_ALG_MATRIX_CLUSTERS
      matrix_clusters src_clusters;
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
      utilz::graphs::io::scan_graph(graph_format, src_graph_fs, src_matrix, std::get<1>(params));
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
      utilz::communities::io::scan_communities(communities_format, src_communities_fs, src_clusters);
      utilz::graphs::io::scan_graph(graph_format, src_graph_fs, src_matrix, src_clusters);
#endif

#ifdef APSP_ALG_MATRIX
      utilz::graphs::io::scan_graph(graph_format, src_graph_fs, src_matrix);
#endif

      this->m_src.push_back(std::move(src_matrix));

#ifdef APSP_ALG_MATRIX_CLUSTERS
      this->m_src_clusters.push_back(std::move(src_clusters));
#endif
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

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_EXTRA_REARRANGEMENTS
    matrix_rearrange src_rearrange;

    src_rearrange(this->m_src[src_index], this->m_src_clusters[src_index], utilz::procedures::matrix_rearrangement_variant::matrix_rearrangement_forward);
  #endif
#endif

#ifdef APSP_ALG_EXTRA_OPTIONS
    auto options = up(this->m_src[src_index], this->m_buf);
#endif

    auto start = std::chrono::high_resolution_clock::now();

#ifdef APSP_ALG_EXTRA_OPTIONS
    run(this->m_src[src_index], options);
#else
    run(this->m_src[src_index]);
#endif

    auto end = std::chrono::high_resolution_clock::now();

    auto elapsed_seconds =
      std::chrono::duration_cast<std::chrono::duration<double>>(
        end - start);

    state.SetIterationTime(elapsed_seconds.count());

#ifdef APSP_ALG_EXTRA_OPTIONS
    down(this->m_src[src_index], this->m_buf, options);
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_EXTRA_REARRANGEMENTS
    src_rearrange(this->m_src[src_index], this->m_src_clusters[src_index], utilz::procedures::matrix_rearrangement_variant::matrix_rearrangement_backward);
  #endif
#endif
  }
}

BENCHMARK_REGISTER_F(Fixture, ExecuteInt)
  ->DenseRange(0, parameters.size() - 1, 1)
  ->UseManualTime()
  ->DisplayAggregatesOnly()
  ->Repetitions(10);

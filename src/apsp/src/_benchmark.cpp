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
#include "memory.hpp"

// local includes
//
#include "algorithm.hpp"

#ifdef APSP_ALG_MATRIX
const auto parameters = std::array<std::tuple<std::string>, 2>({ "10-14.source.g", "32-376.source.g" });
#endif

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

template<typename T>
class Fixture : public benchmark::Fixture
{
public:
// aliasing
//
#ifdef APSP_ALG_EXTRA_CONFIGURATION
  using buffer = ::utilz::memory::buffer_dyn;
#endif

// define global types
//
using g_calculation_type = T;

template<typename K>
using g_allocator_type = typename std::allocator<K>;

// aliasing
//
#ifdef APSP_ALG_MATRIX
  using matrix = ::utilz::matrices::square_matrix<g_calculation_type, g_allocator_type<g_calculation_type>>;

  #ifdef APSP_ALG_EXTRA_CONFIGURATION
  using extra_configuration = run_configuration<g_calculation_type, g_allocator_type<g_calculation_type>>;
  #endif
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
  using matrix_block = ::utilz::matrices::square_matrix<g_calculation_type, g_allocator_type<g_calculation_type>>;
  using matrix       = ::utilz::matrices::square_matrix<matrix_block, g_allocator_type<matrix_block>>;

  #ifdef APSP_ALG_EXTRA_CONFIGURATION
  using extra_configuration = run_configuration<g_calculation_type, g_allocator_type<g_calculation_type>, g_allocator_type<matrix_block>>;
  #endif
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  using matrix_block            = ::utilz::matrices::rect_matrix<g_calculation_type, g_allocator_type<g_calculation_type>>;
  using matrix                  = ::utilz::matrices::square_matrix<matrix_block, g_allocator_type<matrix_block>>;
  using clusters                = ::utilz::matrices::clusters;
  using matrix_arrange_clusters = ::utilz::matrices::procedures::matrix_arrange_clusters<matrix>;

  #ifdef APSP_ALG_EXTRA_CONFIGURATION
  using extra_configuration     = run_configuration<g_calculation_type, g_allocator_type<g_calculation_type>, g_allocator_type<matrix_block>>;
  #endif
#endif

public:
#ifdef APSP_ALG_EXTRA_CONFIGURATION
  buffer               m_buf;
  extra_configuration  m_run_config;
#endif

  std::vector<matrix> m_src;

#ifdef APSP_ALG_MATRIX_CLUSTERS
  std::vector<clusters> m_src_clusters;
#endif

  Fixture()
  {
    ::utilz::graphs::io::graph_format graph_format = ::utilz::graphs::io::graph_format::graph_fmt_weightlist;

#ifdef APSP_ALG_MATRIX_CLUSTERS
    utilz::communities::io::communities_format communities_format = utilz::communities::io::communities_format::communities_fmt_rlang;
#endif

    std::filesystem::path root_path = workspace::root();
    std::filesystem::path data_path = "data/_test/graphs";

    for (auto params : parameters) {
      std::filesystem::path src_graph_path = root_path / data_path / std::get<0>(params);

#ifdef APSP_ALG_MATRIX_CLUSTERS
      std::filesystem::path src_communities_path = root_path / data_path / std::get<1>(params);
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
      clusters src_clusters;
#endif

#ifdef APSP_ALG_MATRIX
      ::utilz::matrices::io::scan_matrix(graph_format, src_graph_fs, src_matrix);
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
      ::utilz::matrices::io::scan_matrix(graph_format, src_graph_fs, src_matrix, std::get<1>(params));
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
      ::utilz::matrices::io::scan_matrix(graph_format, src_graph_fs, communities_format, src_communities_fs, src_matrix, src_clusters);
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
  #ifdef APSP_ALG_EXTRA_CLUSTERS_CONFIGURATION
    up_clusters(this->m_src_clusters[src_index]);
  #endif

    this->m_src_clusters[src_index].optimise();

  #ifdef APSP_ALG_EXTRA_CLUSTERS_REARRANGEMENTS
    matrix_arrange_clusters src_rearrange;

    src_rearrange(this->m_src[src_index], this->m_src_clusters[src_index], ::utilz::matrices::procedures::matrix_clusters_arrangement::matrix_clusters_arrangement_forward);
  #endif
#endif

#ifdef APSP_ALG_EXTRA_CONFIGURATION
    up(this->m_src[src_index], this->m_buf, this->m_run_config);
#endif

    auto start = std::chrono::high_resolution_clock::now();

#ifdef APSP_ALG_EXTRA_CONFIGURATION
  #ifdef APSP_ALG_MATRIX_CLUSTERS
    run(this->m_src[src_index], this->m_run_config, this->m_src_clusters[src_index]);
  #else
    run(this->m_src[src_index], this->m_run_config);
  #endif
#else
  #ifdef APSP_ALG_MATRIX_CLUSTERS
    run(this->m_src[src_index], this->m_src_clusters[src_index]);
  #else
    run(this->m_src[src_index]);
  #endif
#endif

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

    state.SetIterationTime(elapsed_seconds.count());

#ifdef APSP_ALG_EXTRA_CONFIGURATION
    down(this->m_src[src_index], this->m_buf, this->m_run_config);
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_EXTRA_CLUSTERS_REARRANGEMENTS
    src_rearrange(
      this->m_src[src_index],
      this->m_src_clusters[src_index],
      ::utilz::matrices::procedures::matrix_clusters_arrangement::matrix_clusters_arrangement_backward);
  #endif
#endif
  }
}

BENCHMARK_REGISTER_F(Fixture, ExecuteInt)
  ->DenseRange(0, parameters.size() - 1, 1)
  ->UseManualTime()
  ->DisplayAggregatesOnly()
  ->Repetitions(10);

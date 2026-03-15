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
#include "memory.hpp"
#include "measure.hpp"
#include "graphs-io.hpp"

#include "matrix.hpp"
#include "matrix-manip.hpp"
#include "matrix-traits.hpp"
#include "matrix-io.hpp"
#include "matrix-access.hpp"

// local includes
//
#include "_shell_inject.hpp"

#ifdef APSP_ALG_MATRIX_FLAT
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

using size_type  = typename utzmx::traits::matrix_traits<matrix_type>::size_type;
using value_type = typename utzmx::traits::matrix_traits<matrix_type>::value_type;

using buffer_type             = ::utilz::memory::buffer_dyn;
using graph_type              = typename std::tuple<size_type, std::vector<std::tuple<size_type, size_type, value_type>>>;
using communities_type        = typename std::map<size_type, std::vector<size_type>>;
using scan_matrix_params_type = utzmx::io::scan_matrix_params<matrix_type>;

using graph_format_type       = ::utilz::graphs::io::graph_format;
using communities_format_type = ::utilz::communities::io::communities_format;

class Fixture : public benchmark::Fixture
{
public:
  buffer_type m_buffer_fx;

  std::vector<matrix_type>            m_src;
  std::vector<matrix_params_type>     m_src_params;
  std::vector<matrix_clusters_type>   m_src_clusters;
  std::vector<matrix_run_config_type> m_src_run_config;

  Fixture()
  {
    std::filesystem::path root_path = workspace::root();
    std::filesystem::path data_path = "data/_test/graphs";

    for (auto params : parameters) {
      std::filesystem::path graph_path  = root_path / data_path / std::get<0>(params);

#ifdef APSP_ALG_MATRIX_BLOCKS
      size_type block_size              = std::get<1>(params);
#endif

      std::ifstream graph_fs(graph_path);
      if (!graph_fs.is_open())
        throw std::logic_error("erro: the file '" + graph_path.generic_string() + "' doesn't exist.");

      graph_format_type graph_format = graph_format_type::graph_fmt_weightlist;
      graph_type        graph        = ::utilz::graphs::io::scan_graph<size_type, value_type>(graph_format, graph_fs);

#ifdef APSP_ALG_MATRIX_CLUSTERS
      communities_format_type communities_format   = communities_format_type::communities_fmt_rlang;
      std::filesystem::path   communities_path     = root_path / data_path / std::get<1>(params);

      std::ifstream communities_fs(communities_path);
      if (!communities_fs.is_open())
        throw std::logic_error("erro: the file '" + communities_path.generic_string() + "' doesn't exist.");

      communities_type communities = ::utilz::communities::io::scan_communities<size_type>(communities_format, communities_fs);
#endif

#ifdef APSP_ALG_MATRIX_FLAT
      scan_matrix_params_type scan_matrix_params(this->m_buffer_fx, graph);
#endif
#ifdef APSP_ALG_MATRIX_BLOCKS
      scan_matrix_params_type scan_matrix_params(this->m_buffer_fx, graph, block_size);
#endif
#ifdef APSP_ALG_MATRIX_CLUSTERS
      scan_matrix_params_type scan_matrix_params(this->m_buffer_fx, graph, communities);
#endif

#ifdef APSP_ALG_ACCESS_FLAT
      matrix_params_type matrix_params;
#endif
#ifdef APSP_ALG_ACCESS_BLOCKS
      matrix_params_type matrix_params(block_size);
#endif
#ifdef APSP_ALG_ACCESS_CLUSTERS
      matrix_params_type matrix_params(communities);
#endif

      matrix_type        matrix;

      scan_init_matrix(matrix, scan_matrix_params);

      matrix_access_type matrix_access(matrix, matrix_params);

      scan_set_matrix (matrix_access, scan_matrix_params);

#ifdef APSP_ALG_MATRIX_CLUSTERS
      matrix_clusters_type matrix_clusters;
      scan_matrix_clusters(matrix_clusters, scan_matrix_params);
#endif

      this->m_src.push_back(std::move(matrix));
      this->m_src_params.push_back(std::move(matrix_params));
      this->m_src_clusters.push_back(std::move(matrix_clusters));
      this->m_src_run_config.push_back(matrix_run_config_type());
    }
  }
  ~Fixture()
  {
  }
};

BENCHMARK_DEFINE_F(Fixture, Execute)
(benchmark::State& state)
{
  for (auto _ : state) {
    auto src_index = state.range(0);

    matrix_type&            matrix             = this->m_src[src_index];
    matrix_params_type&     matrix_params_type = this->m_src_params[src_index];
    matrix_clusters_type&   matrix_clusters    = this->m_src_clusters[src_index];
    matrix_run_config_type& matrix_run_config  = this->m_src_run_config[src_index];

    matrix_access_type matrix_access(matrix, matrix_params_type);

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_MATRIX_CLUSTERS_CONFIGURATION
    up_clusters(this->m_src_clusters[src_index]);
  #endif

    this->m_src_clusters[src_index].optimise();

  #ifdef APSP_ALG_MATRIX_CLUSTERS_REARRANGEMENTS
    matrix_arrange_procedure_type matrix_arrange_procedure;
    matrix_arrange_procedure(
      matrix_access,
      matrix_clusters,
      ::utilz::matrices::procedures::matrix_arrangement::matrix_arrangement_forward);
  #endif
#endif

#ifdef APSP_ALG_RUN_CONFIGURATION
    up(matrix, matrix_access_type, matrix_run_config, this->m_buffer_fx);
#endif

    auto start = std::chrono::high_resolution_clock::now();

    SHELL_RUN(matrix, matrix_clusters, matrix_run_config);

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

    state.SetIterationTime(elapsed_seconds.count());

#ifdef APSP_ALG_RUN_CONFIGURATION
    down(matrix, matrix_access_type, matrix_run_config, this->m_buffer_fx);
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_MATRIX_CLUSTERS_REARRANGEMENTS
    matrix_arrange_procedure(
      matrix_access,
      matrix_clusters,
      ::utilz::matrices::procedures::matrix_arrangement::matrix_arrangement_backward);
  #endif
#endif
  }
}

BENCHMARK_REGISTER_F(Fixture, Execute)
  ->DenseRange(0, parameters.size() - 1, 1)
  ->UseManualTime()
  ->DisplayAggregatesOnly()
  ->Repetitions(10);

// gtest
//
#include "gtest/gtest.h"

// global includes
//
#include <filesystem>
#include <fstream>

// local internals
//
#include "workspace.hpp"

// local utilz
#include "communities-io.hpp"
#include "graphs-io.hpp"
#include "matrix-io.hpp"
#include "matrix-manip.hpp"
#include "matrix-traits.hpp"
#include "matrix.hpp"
#include "matrix-abstract.hpp"
#include "memory.hpp"

// local includes
//
#include "_shell_inject.hpp"

using buffer_type      = ::utilz::memory::buffer_dyn;

class Fixture : public ::testing::Test
{
public:
  using src_matrix_type             = matrix_type;
  using src_matrix_access_type      = matrix_access_type;
  using src_matrix_params_type      = matrix_params_type;
  using src_matrix_clusters_type    = matrix_clusters_type;
  using src_matrix_run_config_type  = matrix_run_config_type;
  using src_scan_matrix_params_type = scan_matrix_params_type;

  using res_matrix_type             = utzmx::square_matrix<g_type, g_allocator_type<g_type>>;
  using res_matrix_access_type      = utzmx::access::matrix_access<utzmx::access::matrix_access_schema_flat, res_matrix_type>;
  using res_matrix_params_type      = utzmx::access::matrix_params<res_matrix_type>;
  using res_scan_matrix_params_type = utzmx::io::scan_matrix_params<res_matrix_type>;

public:
  buffer_type m_buffer_fx;

  src_matrix_type m_src;
  res_matrix_type m_res;

  src_matrix_params_type m_src_params;
  res_matrix_params_type m_res_params;

  src_matrix_clusters_type   m_src_clusters;
  src_matrix_run_config_type m_src_run_config;

#ifdef APSP_ALG_MATRIX_FLAT
  Fixture(
    const std::string& graph_name)
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
  Fixture(
    const std::string& graph_name,
    const size_type   block_size)
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  Fixture(
    const std::string& graph_name)
#endif
  {
    std::filesystem::path root_path = workspace::root();
    std::filesystem::path data_path = "data/_test/graphs";

    std::filesystem::path src_path = root_path / data_path / (graph_name + ".source.g");
    std::filesystem::path res_path = root_path / data_path / (graph_name + ".result.g");

    std::ifstream src_fs(src_path);
    if (!src_fs.is_open())
    throw std::logic_error("erro: the file '" + src_path.generic_string() + "' doesn't exist.");

    std::ifstream res_fs(res_path);
    if (!res_fs.is_open())
    throw std::logic_error("erro: the file '" + res_path.generic_string() + "' doesn't exist.");

    graph_format_type graph_format = graph_format_type::graph_fmt_weightlist;
    graph_type        src_graph    = ::utilz::graphs::io::scan_graph<size_type, value_type>(graph_format, src_fs);
    graph_type        res_graph    = ::utilz::graphs::io::scan_graph<size_type, value_type>(graph_format, res_fs);

#ifdef APSP_ALG_MATRIX_CLUSTERS
    communities_format_type communities_format   = communities_format_type::communities_fmt_rlang;
    std::filesystem::path   src_communities_path = root_path / data_path / (graph_name + ".communities.g");

    std::ifstream src_communities_fs(src_communities_path);
    if (!src_communities_fs.is_open())
      throw std::logic_error("erro: the file '" + src_communities_path.generic_string() + "' doesn't exist.");

    communities_type src_communities = ::utilz::communities::io::scan_communities<size_type>(communities_format, src_communities_fs);
#endif

#ifdef APSP_ALG_MATRIX_FLAT
    src_scan_matrix_params_type src_scan_matrix_params(this->m_buffer_fx, src_graph);
#endif
#ifdef APSP_ALG_MATRIX_BLOCKS
    src_scan_matrix_params_type src_scan_matrix_params(this->m_buffer_fx, src_graph, block_size);
#endif
#ifdef APSP_ALG_MATRIX_CLUSTERS
    src_scan_matrix_params_type src_scan_matrix_params(this->m_buffer_fx, src_graph, src_communities);
#endif

    res_scan_matrix_params_type res_scan_matrix_params(this->m_buffer_fx, res_graph);

#ifdef APSP_ALG_ACCESS_FLAT
    src_matrix_params_type src_params;
#endif
#ifdef APSP_ALG_ACCESS_BLOCKS
    src_matrix_params_type src_params(block_size);
#endif
#ifdef APSP_ALG_ACCESS_CLUSTERS
    src_matrix_params_type src_params(src_communities);
#endif
    res_matrix_params_type res_params;

    this->m_src_params = src_params;
    this->m_res_params = res_params;

    scan_init_matrix(this->m_src, src_scan_matrix_params);
    scan_init_matrix(this->m_res, res_scan_matrix_params);

    src_matrix_access_type src_matrix_access(this->m_src, this->m_src_params);
    res_matrix_access_type res_matrix_access(this->m_res, this->m_res_params);

    scan_set_matrix(src_matrix_access, src_scan_matrix_params);
    scan_set_matrix(res_matrix_access, res_scan_matrix_params);

#ifdef APSP_ALG_MATRIX_CLUSTERS
    scan_matrix_clusters(this->m_src_clusters, src_scan_matrix_params);
#endif
  };
  ~Fixture(){};

  void
  invoke()
  {
    src_matrix_access_type src_access(this->m_src, this->m_src_params);
    res_matrix_access_type res_access(this->m_res, this->m_res_params);

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_MATRIX_CLUSTERS_CONFIGURATION
    up_clusters(this->m_src_clusters);
  #endif

  this->m_src_clusters.optimise();

  #ifdef APSP_ALG_MATRIX_CLUSTERS_REARRANGEMENTS
    matrix_arrange_procedure_type src_matrix_arrange_procedure;
    src_matrix_arrange_procedure(
      src_access,
      this->m_src_clusters,
      ::utilz::matrices::procedures::matrix_arrangement::matrix_arrangement_forward);
  #endif
#endif

#ifdef APSP_ALG_RUN_CONFIGURATION
    up(this->m_src, src_access, this->m_src_run_config, this->m_buffer_fx);
#endif

    SHELL_RUN(this->m_src, this->m_src_clusters, this->m_src_run_config);

#ifdef APSP_ALG_RUN_CONFIGURATION
    down(this->m_src, src_access, this->m_src_run_config, this->m_buffer_fx);
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_MATRIX_CLUSTERS_REARRANGEMENTS
    src_matrix_arrange_procedure(
      src_access,
      this->m_src_clusters,
      ::utilz::matrices::procedures::matrix_arrangement::matrix_arrangement_backward);
  #endif
#endif

    auto src_dimensions = src_access.dimensions();
    auto res_dimensions = res_access.dimensions();

    for (auto i = size_type(0); i < src_dimensions.h() && i < res_dimensions.h(); ++i)
      for (auto j = size_type(0); j < src_dimensions.w() && j < res_dimensions.w(); ++j)
        ASSERT_EQ(src_access.at(i, j), res_access.at(i, j)) << "  indexes are: [" << i << "," << j << "]";
  }
};

const auto graphs = testing::Values("7-7 (2)", "7-7 (3)", "7-7", "8-9", "9-12 (2)", "9-12", "10-14", "10-36", "17-46", "17-48", "17-57", "17-61 (2)", "17-61", "17-67", "32-376", "32-376 (2)");

#ifdef APSP_ALG_MATRIX_FLAT
const auto values = graphs;

class FixtureP
  : public Fixture
  , public ::testing::WithParamInterface<std::string>
{
public:
  FixtureP()
    : Fixture(GetParam())
  {
  }
};

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, values);
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
const auto values = testing::Combine(graphs, testing::Values(2, 4, 5));

class FixtureP
  : public Fixture
  , public ::testing::WithParamInterface<std::tuple<std::string, int>>
{
public:
  FixtureP()
    : Fixture(std::get<0>(GetParam()), std::get<1>(GetParam()))
  {
  }
};

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, values);
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
const auto values = graphs;

class FixtureP
  : public Fixture
  , public ::testing::WithParamInterface<std::string>
{
public:
  FixtureP()
    : Fixture(GetParam())
  {
  }
};

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, values);
#endif

TEST_P(FixtureP, correctness)
{
  this->invoke();
};

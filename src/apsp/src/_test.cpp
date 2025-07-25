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
#include "algorithm.hpp"

template<typename T>
class Fixture : public ::testing::Test
{
public:
// aliasing
//
using g_calculation_type = T;

template<typename K>
using g_allocator_type = typename std::allocator<K>;

#ifdef APSP_ALG_EXTRA_CONFIGURATION
  using buffer = ::utilz::memory::buffer_dyn;
#endif

#ifdef APSP_ALG_MATRIX
  using source_matrix       = ::utilz::matrices::square_matrix<g_calculation_type, g_allocator_type<g_calculation_type>>;

  #ifdef APSP_ALG_EXTRA_CONFIGURATION
  using extra_configuration = run_configuration<g_calculation_type, g_allocator_type<g_calculation_type>>;
  #endif
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
  using source_matrix_block = ::utilz::matrices::square_matrix<g_calculation_type, g_allocator_type<g_calculation_type>>;
  using source_matrix       = ::utilz::matrices::square_matrix<source_matrix_block, g_allocator_type<source_matrix_block>>;

  #ifdef APSP_ALG_EXTRA_CONFIGURATION
  using extra_configuration = run_configuration<g_calculation_type, g_allocator_type<g_calculation_type>, g_allocator_type<source_matrix_block>>;
  #endif
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  using source_matrix_block    = ::utilz::matrices::rect_matrix<g_calculation_type, g_allocator_type<g_calculation_type>>;
  using source_matrix          = ::utilz::matrices::square_matrix<source_matrix_block, g_allocator_type<source_matrix_block>>;
  using source_clusters        = ::utilz::matrices::clusters;

  #ifdef APSP_ALG_EXTRA_CONFIGURATION
  using extra_configuration    = run_configuration<g_calculation_type, g_allocator_type<g_calculation_type>, g_allocator_type<source_matrix_block>>;
  #endif
#endif

  using size_type = typename ::utilz::matrices::traits::matrix_traits<source_matrix>::size_type;

  using source_matrix_abstract = ::utilz::matrices::matrix_abstract<source_matrix>;
  using source_matrix_arrange  = ::utilz::matrices::procedures::abstract_arrange<source_matrix_abstract>;

  using result_matrix          = ::utilz::matrices::square_matrix<T>;
  using result_matrix_abstract = ::utilz::matrices::matrix_abstract<result_matrix>;

public:
#ifdef APSP_ALG_EXTRA_CONFIGURATION
  buffer m_buf;
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  source_clusters m_src_clusters;
#endif

  source_matrix m_src;
  result_matrix m_res;

#ifdef APSP_ALG_MATRIX
  Fixture(
    const std::string& graph_name)
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
  Fixture(
    const std::string& graph_name,
    const size_type    block_size)
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  Fixture(
    const std::string& graph_name)
#endif
  {
    ::utilz::graphs::io::graph_format graph_format = ::utilz::graphs::io::graph_format::graph_fmt_weightlist;

#ifdef APSP_ALG_MATRIX_CLUSTERS
    ::utilz::communities::io::communities_format communities_format = ::utilz::communities::io::communities_format::communities_fmt_rlang;
#endif

    std::filesystem::path root_path = workspace::root();
    std::filesystem::path data_path = "data/_test/graphs";

    std::filesystem::path src_path = root_path / data_path / (graph_name + ".source.g");

#ifdef APSP_ALG_MATRIX_CLUSTERS
    std::filesystem::path src_communities_path = root_path / data_path / (graph_name + ".communities.g");
#endif

    std::filesystem::path res_path = root_path / data_path / (graph_name + ".result.g");

    std::ifstream src_fs(src_path);
    if (!src_fs.is_open())
      throw std::logic_error("erro: the file '" + src_path.generic_string() + "' doesn't exist.");

#ifdef APSP_ALG_MATRIX_CLUSTERS
    std::ifstream src_communities_fs(src_communities_path);
    if (!src_communities_fs.is_open())
      throw std::logic_error("erro: the file '" + src_communities_path.generic_string() + "' doesn't exist.");
#endif

    std::ifstream res_fs(res_path);
    if (!res_fs.is_open())
      throw std::logic_error("erro: the file '" + res_path.generic_string() + "' doesn't exist.");

    source_matrix_abstract src_abstract(this->m_src);
    result_matrix_abstract res_abstract(this->m_res);

#ifdef APSP_ALG_MATRIX
    ::utilz::matrices::io::scan_matrix(graph_format, src_fs, src_abstract);
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
    ::utilz::matrices::io::scan_matrix(graph_format, src_fs, src_abstract, block_size);
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
    ::utilz::matrices::io::scan_matrix(graph_format, src_fs, communities_format, src_communities_fs, src_abstract, this->m_src_clusters);
#endif

    ::utilz::matrices::io::scan_matrix(graph_format, res_fs, res_abstract);
  };
  ~Fixture(){};

  void
  invoke()
  {
    source_matrix_abstract src_abstract(this->m_src);

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_EXTRA_CLUSTERS_CONFIGURATION
    up_clusters(this->m_src_clusters);
  #endif

  this->m_src_clusters.optimise();

  #ifdef APSP_ALG_EXTRA_CLUSTERS_REARRANGEMENTS
    source_matrix_arrange src_rearrange;

    src_rearrange(
      src_abstract,
      this->m_src_clusters,
      ::utilz::matrices::procedures::matrix_clusters_arrangement::matrix_clusters_arrangement_forward);
  #endif
#endif

#ifdef APSP_ALG_EXTRA_CONFIGURATION
    extra_configuration run_config;
    up(src_abstract, this->m_buf, run_config);
#endif

#ifdef APSP_ALG_EXTRA_CONFIGURATION
  #ifdef APSP_ALG_MATRIX_CLUSTERS
    run(this->m_src, run_config, this->m_src_clusters);
  #else
    run(this->m_src, run_config);
  #endif
#else
  #ifdef APSP_ALG_MATRIX_CLUSTERS
    run(this->m_src, this->m_src_clusters);
  #else
    run(this->m_src);
  #endif
#endif

#ifdef APSP_ALG_EXTRA_CONFIGURATION
    down(src_abstract, this->m_buf, run_config);
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_EXTRA_CLUSTERS_REARRANGEMENTS
    src_rearrange(
      src_abstract,
      this->m_src_clusters,
      ::utilz::matrices::procedures::matrix_clusters_arrangement::matrix_clusters_arrangement_backward);
  #endif
#endif

    for (auto i = size_type(0); i < src_abstract.size() && i < this->m_res.size(); ++i)
      for (auto j = size_type(0); j < src_abstract.size() && j < this->m_res.size(); ++j)
        ASSERT_EQ(src_abstract.at(i, j), this->m_res.at(i, j)) << "  indexes are: [" << i << "," << j << "]";
  }
};

using FixtureT = Fixture<int>;

const auto graphs = testing::Values("7-7 (2)", "7-7 (3)", "7-7", "8-9", "9-12 (2)", "9-12", "10-14", "10-36", "17-46", "17-48", "17-57", "17-61 (2)", "17-61", "17-67", "32-376");

#ifdef APSP_ALG_MATRIX
const auto values = graphs;

class FixtureP
  : public FixtureT
  , public ::testing::WithParamInterface<std::string>
{
public:
  FixtureP()
    : FixtureT(GetParam())
  {
  }
};

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, values);
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
const auto values = testing::Combine(graphs, testing::Values(2, 4, 5));

class FixtureP
  : public FixtureT
  , public ::testing::WithParamInterface<std::tuple<std::string, int>>
{
public:
  FixtureP()
    : FixtureT(std::get<0>(GetParam()), std::get<1>(GetParam()))
  {
  }
};

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, values);
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
const auto values = graphs;

class FixtureP
  : public FixtureT
  , public ::testing::WithParamInterface<std::string>
{
public:
  FixtureP()
    : FixtureT(GetParam())
  {
  }
};

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, values);
#endif

TEST_P(FixtureP, correctness)
{
  this->invoke();
};

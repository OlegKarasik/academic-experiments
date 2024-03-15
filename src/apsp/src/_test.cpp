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

// local includes
//
#include "algorithm.hpp"

template<typename T>
class Fixture : public ::testing::Test
{
public:
// aliasing
//
#ifdef APSP_ALG_EXTRA_OPTIONS
  using buffer = utilz::memory::buffer_dyn;
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
  using source_matrix = utilz::square_matrix<utilz::square_matrix<T>>;
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  using source_matrix = utilz::square_matrix<utilz::rect_matrix<T>>;
#endif

#ifdef APSP_ALG_MATRIX
  using source_matrix = utilz::square_matrix<T>;
#endif

  using size_type = typename utilz::traits::matrix_traits<source_matrix>::size_type;

#ifdef APSP_ALG_MATRIX_CLUSTERS
  using source_clusters = utilz::matrix_clusters<size_type>;
#endif

  using source_matrix_get_at         = utilz::procedures::matrix_at<source_matrix>;
  using source_matrix_get_dimensions = utilz::procedures::matrix_get_dimensions<source_matrix>;
  using source_matrix_rearrange      = utilz::procedures::matrix_rearrange<source_matrix>;

  using result_matrix                = utilz::square_matrix<T>;
  using result_matrix_get_at         = utilz::procedures::matrix_at<result_matrix>;
  using result_matrix_get_dimensions = utilz::procedures::matrix_get_dimensions<result_matrix>;

public:
#ifdef APSP_ALG_EXTRA_OPTIONS
  buffer m_buf;
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  source_clusters m_src_clusters;
#endif

  source_matrix m_src;
  result_matrix m_res;

#ifdef APSP_ALG_MATRIX_BLOCKS
  Fixture(
    const std::string& graph_name,
    const size_type    block_size)
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
    Fixture(
      const std::string& graph_name,
      const std::string& communities_name)
#endif

#ifdef APSP_ALG_MATRIX
      Fixture(
        const std::string& graph_name)
#endif

  {
    utilz::graphs::io::graph_format graph_format = utilz::graphs::io::graph_format::graph_fmt_weightlist;

#ifdef APSP_ALG_MATRIX_CLUSTERS
    utilz::communities::io::communities_format communities_format = utilz::communities::io::communities_format::communities_fmt_rlang;
#endif

    std::filesystem::path root_path = workspace::root();
    std::filesystem::path data_path = "data/_test/direct-acyclic-graphs";

    std::filesystem::path src_path = root_path / data_path / (graph_name + ".source.g");

#ifdef APSP_ALG_MATRIX_CLUSTERS
    std::filesystem::path src_communities_path = root_path / data_path / (communities_name + ".communities.g");
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

#ifdef APSP_ALG_MATRIX_BLOCKS
    utilz::graphs::io::scan_graph(graph_format, src_fs, this->m_src, block_size);
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
    utilz::communities::io::scan_communities(communities_format, src_communities_fs, this->m_src_clusters);
    utilz::graphs::io::scan_graph(graph_format, src_fs, this->m_src, this->m_src_clusters);
#endif

#ifdef APSP_ALG_MATRIX
    utilz::graphs::io::scan_graph(graph_format, src_fs, this->m_src);
#endif

    utilz::graphs::io::scan_graph(graph_format, res_fs, this->m_res);
  };
  ~Fixture(){};

  void
  invoke()
  {
    source_matrix_get_at         src_get_at;
    source_matrix_get_dimensions src_get_dimensions;

    result_matrix_get_at         res_get_at;
    result_matrix_get_dimensions res_get_dimensions;

#ifdef APSP_ALG_MATRIX_CLUSTERS
#ifdef APSP_ALG_EXTRA_REARRANGEMENTS
    source_matrix_rearrange src_rearrange;

    src_rearrange(this->m_src, this->m_src_clusters, utilz::procedures::matrix_rearrangement_variant::matrix_rearrangement_forward);
#endif
#endif

#ifdef APSP_ALG_EXTRA_OPTIONS
    auto options = up(this->m_src, this->m_buf);

    run(this->m_src, options);

    down(this->m_src, this->m_buf, options);
#else
    run(this->m_src);
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
#ifdef APSP_ALG_EXTRA_REARRANGEMENTS
    src_rearrange(this->m_src, this->m_src_clusters, utilz::procedures::matrix_rearrangement_variant::matrix_rearrangement_backward);
#endif
#endif

    auto src_dimensions = src_get_dimensions(this->m_src);
    auto res_dimensions = res_get_dimensions(this->m_res);

    for (auto i = size_type(0); i < src_dimensions.s() && i < res_dimensions.s(); ++i)
      for (auto j = size_type(0); j < src_dimensions.s() && j < res_dimensions.s(); ++j)
        ASSERT_EQ(src_get_at(this->m_src, i, j), res_get_at(this->m_res, i, j)) << "  indexes are: [" << i << "," << j << "]";
  }
};

using FixtureT = Fixture<int>;

#ifdef APSP_ALG_MATRIX_BLOCKS
const auto values = testing::Combine(testing::Values("10-14", "32-376"), testing::Values(2, 4, 5));

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
const auto values = testing::Values(
  std::make_tuple("10-14", "10-14"),
  std::make_tuple("32-376", "32-376"));

class FixtureP
  : public FixtureT
  , public ::testing::WithParamInterface<std::tuple<std::string, std::string>>
{
public:
  FixtureP()
    : FixtureT(std::get<0>(GetParam()), std::get<1>(GetParam()))
  {
  }
};

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, values);
#endif

#ifdef APSP_ALG_MATRIX
const auto values = testing::Values("10-14", "32-376");

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

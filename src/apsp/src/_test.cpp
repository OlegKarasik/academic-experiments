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
#ifdef APSP_ALG_HAS_OPTIONS
  using buffer = utilz::memory::buffer_dyn;
#endif

#if defined(APSP_ALG_HAS_BLOCKS)
  using source_matrix = utilz::square_matrix<utilz::square_matrix<T>>;
#elif defined(APSP_ALG_HAS_UNEQUAL_BLOCKS)
  using source_matrix = utilz::square_matrix<utilz::rect_matrix<T>>;
#else
  using source_matrix = utilz::square_matrix<T>;
#endif

  using source_matrix_gt = utilz::procedures::matrix_at<source_matrix>;
  using source_matrix_dm = utilz::procedures::matrix_get_dimensions<source_matrix>;

  using result_matrix    = utilz::square_matrix<T>;
  using result_matrix_gt = utilz::procedures::matrix_at<result_matrix>;
  using result_matrix_dm = utilz::procedures::matrix_get_dimensions<result_matrix>;

  using size_type = typename utilz::traits::matrix_traits<source_matrix>::size_type;

public:
#ifdef APSP_ALG_HAS_OPTIONS
  buffer m_buf;
#endif

  source_matrix m_src;
  result_matrix m_res;

#if defined(APSP_ALG_HAS_BLOCKS)
  Fixture(
    const std::string& graph_name,
    const size_type    block_size)
#elif defined(APSP_ALG_HAS_UNEQUAL_BLOCKS)
  Fixture(
    const std::string&         graph_name,
    const std::vector<size_t>& block_sizes)
#else
  Fixture(
    const std::string& graph_name)
#endif
  {
    utilz::graphs::io::graph_format format = utilz::graphs::io::graph_format::graph_fmt_weightlist;

    std::filesystem::path root_path = workspace::root();
    std::filesystem::path data_path = "data/_test/direct-acyclic-graphs";

    std::filesystem::path src_path = root_path / data_path / (graph_name + ".source.g");
    std::filesystem::path res_path = root_path / data_path / (graph_name + ".result.g");

    std::ifstream src_fs(src_path);
    if (!src_fs.is_open())
      throw std::logic_error("erro: the file '" + src_path.generic_string() + "' doesn't exist.");

    std::ifstream res_fs(res_path);
    if (!res_fs.is_open())
      throw std::logic_error("erro: the file '" + res_path.generic_string() + "' doesn't exist.");

#if defined(APSP_ALG_HAS_BLOCKS)
    utilz::graphs::io::scan_graph(format, src_fs, this->m_src, block_size);
#elif defined(APSP_ALG_HAS_UNEQUAL_BLOCKS)
    utilz::graphs::io::scan_graph(format, src_fs, this->m_src, block_sizes);
#else
    utilz::graphs::io::scan_graph(format, src_fs, this->m_src);
#endif

    utilz::graphs::io::scan_graph(format, res_fs, this->m_res);
  };
  ~Fixture(){};

  void
  invoke()
  {
#ifdef APSP_ALG_HAS_OPTIONS
    auto options = up(this->m_src, this->m_buf);

    run(this->m_src, options);

    down(this->m_src, this->m_buf, options);
#else
    run(this->m_src);
#endif

    source_matrix_gt src_gt;
    source_matrix_dm src_dm;

    result_matrix_gt res_gt;
    result_matrix_dm res_dm;

    auto src_dimensions = src_dm(this->m_src);
    auto res_dimensions = res_dm(this->m_res);

    for (auto i = size_type(0); i < src_dimensions.s() && i < res_dimensions.s(); ++i)
      for (auto j = size_type(0); j < src_dimensions.s() && j < res_dimensions.s(); ++j)
        ASSERT_EQ(src_gt(this->m_src, i, j), res_gt(this->m_res, i, j)) << "  indexes are: [" << i << "," << j << "]";
  }
};

using FixtureT = Fixture<int>;

#if defined(APSP_ALG_HAS_BLOCKS)
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
#elif defined(APSP_ALG_HAS_UNEQUAL_BLOCKS)
const auto values = testing::Values(
  std::make_tuple("10-14", std::vector<size_t>{ 2, 3, 3, 2 }),
  std::make_tuple("32-376", std::vector<size_t>{ 4, 5, 10, 4, 4, 5 }));

class FixtureP
  : public FixtureT
  , public ::testing::WithParamInterface<std::tuple<std::string, std::vector<size_t>>>
{
public:
  FixtureP()
    : FixtureT(std::get<0>(GetParam()), std::get<1>(GetParam()))
  {
  }
};

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, values);
#else
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

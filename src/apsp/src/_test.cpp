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
#include "square-shape.hpp"
#include "shapes-traits.hpp"
#include "shapes-manip.hpp"
#include "shapes-io.hpp"

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

#ifdef APSP_ALG_HAS_BLOCKS
  using matrix = utilz::square_shape<utilz::square_shape<T>>;
#else
  using matrix = utilz::square_shape<T>;
#endif

  using matrix_gt = utilz::procedures::square_shape_get<matrix>;
  using matrix_gw = utilz::procedures::shape_get_width<matrix>;
  using matrix_gh = utilz::procedures::shape_get_height<matrix>;

  using matrix_st = typename utilz::traits::square_shape_traits<matrix>::size_type;
  using matrix_vt = typename utilz::traits::square_shape_traits<matrix>::value_type;

public:
#ifdef APSP_ALG_HAS_OPTIONS
  buffer m_buf;
#endif

  matrix m_src;
  matrix m_res;

#ifdef APSP_ALG_HAS_BLOCKS
  Fixture(
    const std::string& graph_name,
    const matrix_st    block_size)
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

#ifdef APSP_ALG_HAS_BLOCKS
    utilz::graphs::io::scan_graph(format, src_fs, this->m_src, block_size);
    utilz::graphs::io::scan_graph(format, res_fs, this->m_res, block_size);
#else
    utilz::graphs::io::scan_graph(format, src_fs, this->m_src);
    utilz::graphs::io::scan_graph(format, res_fs, this->m_res);
#endif
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

    matrix_gt gt;
    matrix_gw gw;
    matrix_gh gh;

    for (auto i = matrix_st(0); i < gh(this->m_src) && gh(this->m_res); ++i)
      for (auto j = matrix_st(0); j < gw(this->m_src) && gw(this->m_res); ++j)
        ASSERT_EQ(gt(this->m_src, i, j), gt(this->m_res, i, j)) << "  indexes are: [" << i << "," << j << "]";
  }
};

using FixtureT = Fixture<int>;

const auto graph_names = testing::Values("10-14", "32-376");

#ifdef APSP_ALG_HAS_BLOCKS
const auto block_sizes = testing::Values(2, 4, 5);

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

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, testing::Combine(graph_names, block_sizes));
#else
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

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, graph_names);
#endif

TEST_P(FixtureP, correctness)
{
  this->invoke();
};

// global includes
//
#include <filesystem>
#include <fstream>

#include "gtest/gtest.h"

// internals
//
#include "workspace.hpp"

// global utilz
#include "square-shape.hpp"

// local includes
//
#include "algorithm.hpp"
#include "io.hpp"

template<typename T>
class Fixture : public ::testing::Test
{
public:
// aliasing
//
#ifdef APSP_ALG_HAS_OPTIONS
  using buffer = ::utilz::memory::buffer_dyn;
#endif

#ifdef APSP_ALG_HAS_BLOCKS
  using matrix = ::utilz::square_shape<::utilz::square_shape<T>>;
#else
  using matrix = ::utilz::square_shape<T>;
#endif

  using matrix_at = utilz::procedures::square_shape_at<matrix>;
  using matrix_sz = utilz::procedures::square_shape_get_size<matrix>;
  using matrix_st = typename utilz::traits::square_shape_traits<matrix>::size_type;

public:
#ifdef APSP_ALG_HAS_OPTIONS
  buffer m_buf;
#endif

  matrix m_src;
  matrix m_res;

  Fixture(
    const std::string& graph_name)
  {
    std::filesystem::path root_path = workspace::root();
    std::filesystem::path data_path = "data/_test/direct-acyclic-graphs";

    std::filesystem::path src_path  = root_path / data_path / (graph_name + ".source.g");
    std::filesystem::path res_path  = root_path / data_path / (graph_name + ".result.g");

    std::ifstream src_fs(src_path);
    if (!src_fs.is_open())
      throw std::logic_error("erro: the file '" + src_path.generic_string() + "' doesn't exist.");

    std::ifstream res_fs(res_path);
    if (!res_fs.is_open())
      throw std::logic_error("erro: the file '" + res_path.generic_string() + "' doesn't exist.");

#ifdef APSP_ALG_HAS_BLOCKS
    ::apsp::io::scan_matrix(src_fs, false, this->m_src, 5);
    ::apsp::io::scan_matrix(res_fs, false, this->m_res, 5);
#else
    ::apsp::io::scan_matrix(src_fs, false, this->m_src);
    ::apsp::io::scan_matrix(res_fs, false, this->m_res);
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

    matrix_at at;
    matrix_sz sz;

    for (auto i = matrix_st(0); i < sz(this->m_src) && sz(this->m_res); ++i)
      for (auto j = matrix_st(0); j < sz(this->m_src) && sz(this->m_res); ++j)
        ASSERT_EQ(at(this->m_src, i, j), at(this->m_res, i, j));
  }
};

using FixtureT = Fixture<int>;

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

const auto values = testing::Values("10-14", "32-376");

INSTANTIATE_TEST_SUITE_P(FIXTURE_NAME, FixtureP, values);

TEST_P(FixtureP, correctness)
{
  this->invoke();
};

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

public:
#ifdef APSP_ALG_HAS_OPTIONS
  buffer m_buf;
#endif

  matrix m_src;
  matrix m_res;

  Fixture()
  {
    std::filesystem::path root_path = workspace::root();
    std::filesystem::path src_path  = root_path / "data/_test/direct-acyclic-graphs/10-14.source.g";
    std::filesystem::path res_path  = root_path / "data/_test/direct-acyclic-graphs/10-14.result.g";

    std::ifstream src_fs(src_path);
    if (!src_fs.is_open())
      throw std::logic_error("erro: the file '" + src_path.generic_string() + "' doesn't exist.");

    std::ifstream res_fs(res_path);
    if (!res_fs.is_open())
      throw std::logic_error("erro: the file '" + res_path.generic_string() + "' doesn't exist.");

#ifdef APSP_ALG_HAS_BLOCKS
    ::apsp::io::scan_matrix(src_fs, false, this->m_src, 2);
    ::apsp::io::scan_matrix(res_fs, false, this->m_res, 2);
#else
    ::apsp::io::scan_matrix(src_fs, false, this->m_src);
    ::apsp::io::scan_matrix(res_fs, false, this->m_res);
#endif
  };
  ~Fixture(){};
};

using FixtureT = Fixture<int>;

TEST_F(FixtureT, FIXTURE_NAME)
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

  for (matrix::size_type i = matrix::size_type(0); i < sz(this->m_src) && sz(this->m_res); ++i)
    for (matrix::size_type j = matrix::size_type(0); j < sz(this->m_src) && sz(this->m_res); ++j)
      ASSERT_EQ(at(this->m_src, i, j), at(this->m_res, i, j));
};

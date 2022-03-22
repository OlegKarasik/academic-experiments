// global algorithms
//
#if (ALG_VARIATION == 0)
  #include "../algorithms/00.hpp"
#endif

#if (ALG_VARIATION == 1)
  #include "../algorithms/01.hpp"
#endif

#if (ALG_VARIATION == 2)
  #include "../algorithms/02.hpp"
#endif

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

// local utilz
#include "../io.hpp"

template<typename T>
class Fixture : public ::testing::Test
{
public:
// aliasing
//
#ifdef APSP_ALG_WIND_UPDOWN
  using buffer = ::utilz::memory::buffer_dyn;
#endif

#ifdef APSP_ALG_BLOCKED
  using matrix = ::utilz::square_shape<::utilz::square_shape<T>>;
#else
  using matrix = ::utilz::square_shape<T>;
#endif

  using matrix_at = utilz::procedures::square_shape_at<matrix>;

public:
#ifdef APSP_ALG_WIND_UPDOWN
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

#ifdef APSP_ALG_BLOCKED
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

TEST_F(FixtureT, Execute)
{
#ifdef APSP_ALG_WIND_UPDOWN
  auto options = wind_up_apsp(this->m_src, this->m_buf);

  calculate_apsp(this->m_src, options);

  wind_down_apsp(this->m_src, this->m_buf, options);
#else
  calculate_apsp(this->m_src);
#endif

  matrix_at at;

  for (matrix::size_type i = matrix::size_type(0); i < this->m_src.size() && this->m_res.size(); ++i)
    for (matrix::size_type j = matrix::size_type(0); j < this->m_src.size() && this->m_res.size(); ++j)
      ASSERT_EQ(at(this->m_src, i, j), at(this->m_res, i, j));
};

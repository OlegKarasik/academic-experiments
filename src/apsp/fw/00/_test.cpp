#include "gtest/gtest.h"

#include "_internal.hpp"

#include "graphs-io.hpp"
#include "square-shape.hpp"

#include "algorithms.hpp"

#include <filesystem>
#include <fstream>

using namespace utilz;
using namespace graphs::io;

template<typename T>
class Fixture : public ::testing::Test
{
  // Constant value which indicates that there is no path between two vertexes.
  // Please note: this value can be used ONLY when input paths are >= 0.
  //
  static constexpr T
  no_path_value()
  {
    return ((std::numeric_limits<T>::max)() / 2) - 1;
  };

  using matrix          = utilz::square_shape<int>;
  using matrix_sz       = typename utilz::traits::square_shape_traits<matrix>::size_type;
  using matrix_set_size = utilz::procedures::square_shape_set_size<matrix>;
  using matrix_set_at   = utilz::procedures::square_shape_set<matrix>;
  using matrix_get_size = utilz::procedures::square_shape_get_size<matrix>;
  using matrix_get_at   = utilz::procedures::square_shape_get<matrix>;

public:
  matrix m_src_matrix;
  matrix m_res_matrix;

  Fixture()
  {
    std::filesystem::path root_path = _internal::root();
    std::filesystem::path src_path  = root_path / "data/_test/direct-acyclic-graphs/10-14.source.g";
    std::filesystem::path res_path  = root_path / "data/_test/direct-acyclic-graphs/10-14.result.g";

    matrix_set_size set_sz;
    matrix_set_at   set_at;

    std::ifstream src_fs(src_path);
    if (!src_fs.is_open())
      throw std::logic_error("erro: the file '" + src_path.generic_string() + "' doesn't exist.");

    std::ifstream res_fs(res_path);
    if (!res_fs.is_open())
      throw std::logic_error("erro: the file '" + res_path.generic_string() + "' doesn't exist.");

    utilz::graphs::io::scan_matrix(src_fs, this->m_src_matrix, set_sz, set_at);

    for (matrix_sz i = matrix_sz(0); i < this->m_src_matrix.size(); ++i)
      for (matrix_sz j = matrix_sz(0); j < this->m_src_matrix.size(); ++j)
        if (this->m_src_matrix.at(i, j) == 0)
          this->m_src_matrix.at(i, j) = no_path_value();

    utilz::graphs::io::scan_matrix(res_fs, this->m_res_matrix, set_sz, set_at);

    for (matrix_sz i = matrix_sz(0); i < this->m_res_matrix.size(); ++i)
      for (matrix_sz j = matrix_sz(0); j < this->m_res_matrix.size(); ++j)
        if (this->m_res_matrix.at(i, j) == 0)
          this->m_res_matrix.at(i, j) = no_path_value();
  };
  ~Fixture(){};
};

using FixtureT = Fixture<int>;

TEST_F(FixtureT, Execute)
{
  calculate_apsp(this->m_src_matrix);

  for (size_t i = 0; i < this->m_src_matrix.size() && this->m_res_matrix.size(); ++i)
    for (size_t j = 0; j < this->m_src_matrix.size() && this->m_res_matrix.size(); ++j)
      ASSERT_EQ(this->m_src_matrix.at(i, j), this->m_res_matrix.at(i, j));
};

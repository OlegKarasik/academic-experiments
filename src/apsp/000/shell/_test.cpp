#include <filesystem>
#include <fstream>

#include "gtest/gtest.h"

// internals
#include "workspace.hpp"

// global utilz
#include "square-shape.hpp"

// local utilz
#include "../../io.hpp"

// algorithm
#if (ALG_VARIATION == 0)
  #include "../00.hpp"
#endif

#if (ALG_VARIATION == 1)
  #include "../01.hpp"
#endif

namespace apsp  = ::apsp;
namespace utilz = ::utilz;

template<typename T>
class Fixture : public ::testing::Test
{
public:
  using matrix    = utilz::square_shape<int>;
  using matrix_st = typename utilz::traits::square_shape_traits<utilz::square_shape<int>>::size_type;

public:
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

    apsp::io::scan_matrix(src_fs, this->m_src);
    apsp::io::scan_matrix(res_fs, this->m_res);
  };
  ~Fixture(){};
};

using FixtureT = Fixture<int>;

TEST_F(FixtureT, Execute)
{
  calculate_apsp(this->m_src);

  for (matrix_st i = matrix_st(0); i < this->m_src.size() && this->m_res.size(); ++i)
    for (matrix_st j = matrix_st(0); j < this->m_src.size() && this->m_res.size(); ++j)
      ASSERT_EQ(this->m_src.at(i, j), this->m_res.at(i, j));
};

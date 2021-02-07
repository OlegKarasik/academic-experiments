#include "gtest/gtest.h"

#include "../../utilz/graphs-fio.hpp"

#include "../../utilz/square-shape-io.hpp"
#include "../../utilz/square-shape.hpp"

#include "algorithm.hpp"

#include <filesystem>

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

  using shape      = square_shape<T>;
  using resize_op  = square_shape_resize_graph_op<T, no_path_value()>;
  using element_op = square_shape_element_op<T>;

private:
  const std::filesystem::path _s = "../../../../data/_test/direct-acyclic-graphs/10-14.source.g";
  const std::filesystem::path _r = "../../../../data/_test/direct-acyclic-graphs/10-14.result.g";

  resize_op m_resize;

public:
  shape m_source;
  shape m_result;

  Fixture()
  {
    std::filesystem::path source = std::filesystem::current_path() / _s;
    std::filesystem::path result = std::filesystem::current_path() / _r;

    element_op op;

    graphs::io::fscan_graph<shape, T, resize_op, element_op>(source, this->m_source, this->m_resize, op);
    graphs::io::fscan_graph<shape, T, resize_op, element_op>(result, this->m_result, this->m_resize, op);
  };
  ~Fixture(){};
};

using FixtureT = Fixture<int>;

TEST_F(FixtureT, Execute)
{
  impl(this->m_source);

  for (size_t i = 0; i < this->m_source.s() && this->m_result.s(); ++i)
    for (size_t j = 0; j < this->m_source.s() && this->m_result.s(); ++j)
      ASSERT_EQ(this->m_source(i, j), this->m_result(i, j));
};

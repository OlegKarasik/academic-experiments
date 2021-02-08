#pragma once

#include <memory>
#include <random>

namespace utilz {
namespace graphs {

template<typename T, typename DistributionEngine, typename Distribution, typename A = std::allocator<T>>
class square_shape_at_assign_random_weight_operation
{
private:
  DistributionEngine& m_engine;
  Distribution&       m_distribution;

public:
  square_shape_at_assign_random_weight_operation(DistributionEngine& engine, Distribution& distribution)
    : m_engine(engine)
    , m_distribution(distribution)
  {}

  void operator()(square_shape<T, A>& s, size_t i, size_t j)
  {
    s.at(i, j) = this->m_distribution(this->m_engine);
  }
};

} // namespace graphs
} // namespace utilz

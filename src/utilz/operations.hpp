#pragma once

namespace utilz {
namespace operations {

template<typename T, typename DistributionEngine, typename Distribution>
class random_value
{
public:
  using result_type = typename Distribution::result_type;

private:
  DistributionEngine& m_engine;
  Distribution&       m_distribution;

public:
  random_value(DistributionEngine& engine, Distribution& distribution)
    : m_engine(engine)
    , m_distribution(distribution)
  {}

  void operator()(result_type& v)
  {
    v = this->m_distribution(this->m_engine);
  }
};

template<typename DistributionEngine, typename Distribution>
random_value(DistributionEngine& engine, Distribution& distribution)
  -> random_value<typename Distribution::result_type, DistributionEngine, Distribution>;

} // namespace operations
} // namespace utilz

#include "square-shape.hpp"

#include <iostream>
#include <random>
#include <algorithm>

#include "graphs-generators.hpp"
#include "graphs-io.hpp"

template<typename S>
class set_size
{
  static_assert(utilz::traits::square_shape_traits<S>::is::value, "not a square shape");

public:
  using result_type = typename utilz::traits::square_shape_traits<S>::size_type;

public:
  void
  operator()(S& s, typename utilz::traits::square_shape_traits<S>::size_type sz)
  {
    s = S(sz);
  }
};

template<typename S>
class set_value
{
  static_assert(utilz::traits::square_shape_traits<S>::is::value, "not a square shape");

public:
  using result_type = typename utilz::traits::square_shape_traits<S>::value_type;

public:
  void
  operator()(
    S&                                                         s,
    typename utilz::traits::square_shape_traits<S>::size_type  i,
    typename utilz::traits::square_shape_traits<S>::size_type  j,
    typename utilz::traits::square_shape_traits<S>::value_type v)
  {
    s.at(i, j) = v;
  }
};

template<typename S>
class get_size
{
  static_assert(utilz::traits::square_shape_traits<S>::is::value, "not a square shape");

public:
  using result_type = typename utilz::traits::square_shape_traits<S>::size_type;

public:
  typename utilz::traits::square_shape_traits<S>::size_type
  operator()(S& s)
  {
    return s.size();
  }
};

template<typename S>
class get_value
{
  static_assert(utilz::traits::square_shape_traits<S>::is::value, "not a square shape");

public:
  using result_type = typename utilz::traits::square_shape_traits<S>::value_type;

public:
  typename utilz::traits::square_shape_traits<S>::value_type
  operator()(
    S&                                                        s,
    typename utilz::traits::square_shape_traits<S>::size_type i,
    typename utilz::traits::square_shape_traits<S>::size_type j)
  {
    return s.at(i, j);
  }
};

int
main(int argc, char* argv[])
{
  size_t v = 250;
  size_t e = size_t(((v * (v - 1)) / 2) * 0.5);
  size_t h = 30;

  utilz::square_shape<int>            random_adj;
  std::vector<utilz::graphs::generators::promised_path<size_t>> vec;

  std::mt19937_64                       home_engine;
  std::uniform_int_distribution<size_t> home_distribution(size_t(0), (v / 2) - h);
  home_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  std::mt19937_64                       flag_engine;
  std::uniform_int_distribution<size_t> flag_distribution((v / 2), v - 1);
  flag_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  std::mt19937_64                       hops_engine;
  std::uniform_int_distribution<size_t> hops_distribution(25, h);
  flag_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  std::mt19937_64                       amount_engine;
  std::uniform_int_distribution<size_t> amount_distribution(20, 30);
  amount_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  std::mt19937_64                       limit_engine;
  std::uniform_int_distribution<size_t> limit_distribution(20, 40);
  limit_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  size_t amount = amount_distribution(amount_engine);
  size_t home = home_distribution(home_engine);
  size_t limit = limit_distribution(limit_engine);

  for (size_t i = 0; i < amount; ++i)
  {
    vec.emplace_back(home, flag_distribution(flag_engine), hops_distribution(hops_engine));
  }

  set_size<utilz::square_shape<int>>  s_size;
  set_value<utilz::square_shape<int>> s_value;
  utilz::graphs::generators::random_graph(
    v, e, vec, random_adj, s_size, s_value, utilz::graphs::generators::directed_acyclic_graph_tag());

  std::mt19937_64                    distribution_engine;
  std::uniform_int_distribution<int> vertex_distribution(1, 15);

  for (auto i = 0; i < random_adj.size(); ++i)
    for (auto j = 0; j < random_adj.size(); ++j)
      if (random_adj.at(i, j) == 1)
        random_adj.at(i, j) = vertex_distribution(distribution_engine);

  get_size<utilz::square_shape<int>>  g_size;
  get_value<utilz::square_shape<int>> g_value;

  std::cout << limit << std::endl;
  std::cout << home << std::endl;
  for (size_t i = 0; i < vec.size() - 1; ++i)
    std::cout << vec[i].t << " ";

  std::cout << vec[vec.size() - 1].t << std::endl;

  utilz::graphs::io::print_matrix(std::cout, random_adj, g_size, g_value);
}

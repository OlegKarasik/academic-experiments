#include "square-shape.hpp"

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
  size_t v = 4800;
  size_t e = size_t(((v * (v - 1)) / 2) * 0.8);

  std::cout << e << std::endl;

  utilz::square_shape<int>            random_adj;
  set_size<utilz::square_shape<int>>  s_size;
  set_value<utilz::square_shape<int>> s_value;
  utilz::graphs::generators::random_graph(v, e, random_adj, s_size, s_value, utilz::graphs::generators::directed_acyclic_graph_tag());

  std::mt19937_64                    distribution_engine;
  std::uniform_int_distribution<int> vertex_distribution(1, 100);

  for (auto i = 0; i < random_adj.size(); ++i)
    for (auto j = 0; j < random_adj.size(); ++j)
      if (random_adj.at(i, j) == 1)
        random_adj.at(i, j) = vertex_distribution(distribution_engine);

  get_size<utilz::square_shape<int>>  g_size;
  get_value<utilz::square_shape<int>> g_value;

  utilz::graphs::io::print_matrix(std::cout, random_adj, g_size, g_value);
}

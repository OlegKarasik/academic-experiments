#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>

#include "algorithm.hpp"

#include "operations.hpp"

#include "graphs-generators.hpp"

#include "square-shape-graphs.hpp"
#include "square-shape.hpp"

using namespace utilz;

// Constant value which indicates that there is no path between two vertexes.
// Please note: this value can be used ONLY when input paths are >= 0.
//
constexpr int
no_path_value()
{
  return ((std::numeric_limits<int>::max)() / 2) - 1;
};

// A precondition for input matrix to ensure proper algorithm execution.
// Ensures: matrix is a square matrix; all cell values are greater or equal to zero.
//
// template<typename T, T V>
// class rect_shape_precondition
// {
// private:
//   const T m_s = V;

// public:
//   rect_shape_precondition()
//   {}

//   bool operator()(const rect_shape<T>& s)
//   {
//     for (size_t i = 0; i < s.w(); ++i)
//       for (size_t j = 0; j < s.h(); ++j)
//         if (s(i, j) < this->m_s)
//           return false;

//     return s.w() == s.h();
//   }
// };

// using matrix_precondition     = rect_shape_precondition<long, 0>;
// using matrix_memory           = rect_shape_matrix_memory<long, no_path_value()>;
// using matrix_output           = rect_shape_matrix_output<long, matrix_memory>;
// using matrix_output_predicate = matrix_all_predicate<long>;
// using matrix_input            = rect_shape_matrix_input<long>;
// using matrix_input_predicate  = matrix_except_predicate<long, no_path_value()>;

template<typename T, T V>
class except_predicate
{
private:
  const T m_v = V;

public:
  except_predicate()
  {}

  bool
  operator()(const T& v) const
  {
    return v != this->m_v;
  }
};

template<typename S>
class set_value
{
  static_assert(traits::square_shape_traits<S>::is::value, "not a square shape");

public:
  using result_type = typename traits::square_shape_traits<S>::value_type;

public:
  void
  operator()(
    S&                                                  s,
    typename traits::square_shape_traits<S>::size_type  i,
    typename traits::square_shape_traits<S>::size_type  j,
    typename traits::square_shape_traits<S>::value_type v)
  {
    s.at(i, j) = v;
  }
};

template<typename S>
class get_size
{
  static_assert(traits::square_shape_traits<S>::is::value, "not a square shape");

public:
  using result_type = typename traits::square_shape_traits<S>::size_type;

public:
  typename traits::square_shape_traits<S>::size_type
  operator()(S& s)
  {
    return s.size();
  }
};

template<typename S>
class get_value
{
  static_assert(traits::square_shape_traits<S>::is::value, "not a square shape");

public:
  using result_type = typename traits::square_shape_traits<S>::value_type;

public:
  typename traits::square_shape_traits<S>::value_type
  operator()(
    S&                                                 s,
    typename traits::square_shape_traits<S>::size_type i,
    typename traits::square_shape_traits<S>::size_type j)
  {
    return s.at(i, j);
  }
};

int
main(int argc, char* argv[])
{
  std::mt19937_64               distribution_engine;
  std::uniform_int_distribution distribution(1, 10);

  //operations::random_value          rv(distribution_engine, distribution);
  //procedures::square_shape_at       at(rv);

  square_shape<int> s(10);

  s.at(0, 5) = 11;

  square_shape<square_shape<int>> bs(10);
  for (auto i = 0; i < 10; ++i)
    for (auto j = 0; j < 10; ++j)
      bs.at(i, j) = square_shape<int>(5);

  bs.at(0, 1).at(0, 2) = 10;

  procedures::square_shape_size<square_shape<int>>               a;
  procedures::square_shape_size<square_shape<square_shape<int>>> b;

  procedures::square_shape_at<square_shape<int>>               aa;
  procedures::square_shape_at<square_shape<square_shape<int>>> bb;

  size_t r1 = a(s);
  size_t r2 = b(bs);

  int r3 = aa(s, 0, 5);
  int r4 = bb(bs, 0, 12);

  bool b1 = traits::square_shape_traits<int>::is::value;
  bool b2 = traits::square_shape_traits<square_shape<int>>::is::value;
  bool b3 = traits::square_shape_traits<square_shape<square_shape<int>>>::is::value;
  bool b4 = traits::square_shape_traits<square_shape<square_shape<square_shape<int>>>>::is::value;
  bool b5 = traits::square_shape_traits<square_shape<int>::value_type>::is::value;

  auto b6 = typeid(traits::square_shape_traits<square_shape<int>>::value_type).name();
  auto b7 = typeid(traits::square_shape_traits<square_shape<square_shape<int>>>::value_type).name();
  auto b8 = typeid(traits::square_shape_traits<square_shape<square_shape<square_shape<long>>>>::value_type).name();

  square_shape<int>            simple;
  square_shape<square_shape<int>>            max;
  square_shape<square_shape<square_shape<int>>>            huge;
  // set_size<square_shape<int>>  s_size;
  // set_value<square_shape<int>> s_value;
  //graphs::random_dag(10, 15, random_adj, s_size, s_value);


utilz::graphs::square_shape_set_size<int> tuple_trivial;
utilz::graphs::square_shape_set_size<square_shape<int>> tuple_simple;
utilz::graphs::square_shape_set_size<square_shape<square_shape<int>>> tuple_max(5);
utilz::graphs::square_shape_set_size<square_shape<square_shape<square_shape<int>>>> tuple_huge(50, 5);

  tuple_simple(simple, 50);
  tuple_max(max, 500);
  tuple_huge(huge, 500);


  get_size<square_shape<int>>  g_size;
  get_value<square_shape<int>> g_value;

  //graphs::io::cprint_adj_matrix(random_adj, g_size, g_value);

  // for (size_t i = 0; i < random_adj.size(); ++i) {
  //   for (size_t j = 0; j < random_adj.size(); ++j) {
  //     std::cout << "(" << i << "," << j << ") = " << random_adj.at(i, j) << std::endl;
  //   }
  // }
}

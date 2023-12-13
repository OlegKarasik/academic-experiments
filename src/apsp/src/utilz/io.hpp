#pragma once

// global utilz
//
#include "graphs-io.hpp"
#include "square-shape.hpp"

// local utilz
//
#include "constants.hpp"

namespace apsp {
namespace io {

template<typename S, typename... TArgs>
void
scan_matrix(std::istream& s, bool binary, S& m, TArgs... args)
{
  static_assert(::utilz::traits::square_shape_traits<S>::is::value, "erro: input type has to be a square_shape of T");

  using sz_fn  = ::utilz::procedures::square_shape_set_size<S>;
  using set_fn = ::utilz::procedures::square_shape_set<S>;
  using rep_fn = ::utilz::procedures::square_shape_replace<S>;

  using value_type = typename ::utilz::traits::square_shape_traits<S>::value_type;

  sz_fn  sz(args...);
  set_fn set;
  rep_fn rep;

  ::utilz::graphs::io::scan_graph(s, binary, m, sz, set);

  // Replace 'zero' values with corresponding 'infinity' value to ensure correct
  // execution of APSP Floyd-Warshall algorithm
  //
  rep(m, value_type(0), apsp::constants::infinity<value_type>());
};

template<typename S>
void
print_matrix(std::ostream& s, bool binary, S& m)
{
  static_assert(::utilz::traits::square_shape_traits<S>::is::value, "erro: input type has to be a square_shape of T");

  using sz_fn  = ::utilz::procedures::square_shape_get_size<S>;
  using get_fn = ::utilz::procedures::square_shape_get<S>;
  using rep_fn = ::utilz::procedures::square_shape_replace<S>;

  using value_type = typename ::utilz::traits::square_shape_traits<S>::value_type;

  sz_fn  sz;
  get_fn get;
  rep_fn rep;

  // Replace 'infinity' values with corresponding 'zero' values to
  // correctly send them to output
  //
  rep(m, apsp::constants::infinity<value_type>(), value_type(0));

  ::utilz::graphs::io::print_graph(s, binary, m, sz, get);
};

} // namespace io
} // namespace apsp

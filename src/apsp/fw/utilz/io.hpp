#pragma once

#include "graphs-io.hpp"
#include "square-shape.hpp"

#include "constants.hpp"

namespace fw {
namespace utilz {
namespace io {

template<typename T, typename A>
void
scan_matrix(
  std::istream&                                                           s,
  ::utilz::square_shape<T, A>&                                            m,
  ::utilz::procedures::square_shape_set_size<::utilz::square_shape<T, A>> sz)
{
  using set_fn = ::utilz::procedures::square_shape_set<::utilz::square_shape<T, A>>;
  using rep_fn = ::utilz::procedures::square_shape_replace<::utilz::square_shape<T, A>>;

  using valut_type = typename ::utilz::traits::square_shape_traits<::utilz::square_shape<T, A>>::value_type;

  set_fn set;
  rep_fn rep;

  ::utilz::graphs::io::scan_matrix(s, m, sz, set);

  // Replace 'zero' values with corresponding 'infinity' value to ensure correct
  // execution of APSP Floyd-Warshall algorithm
  //
  rep(m, valut_type(0), utilz::constants::infinity<valut_type>());
};

template<typename T, typename A>
void
print_matrix(
  std::ostream&                                                            s,
  ::utilz::square_shape<T, A>&                                             m,
  ::utilz::procedures::square_shape_get_size<::utilz::square_shape<T, A>>& sz)
{
  using get_fn = ::utilz::procedures::square_shape_get<::utilz::square_shape<T, A>>;
  using rep_fn = ::utilz::procedures::square_shape_replace<::utilz::square_shape<T, A>>;

  using value_type = typename ::utilz::traits::square_shape_traits<::utilz::square_shape<T, A>>::value_type;

  get_fn get;
  rep_fn rep;

  // Replace 'infinity' values with corresponding 'zero' values to minimize
  // output size (i.e. 'zero' values aren't send to output stream)
  //
  rep(m, utilz::constants::infinity<value_type>(), value_type(0));

  ::utilz::graphs::io::print_matrix(s, m, sz, get);
};

} // namespace io
} // namespace utilz
} // namespace fw

#pragma once

#include "portables/hacks/defines.h"

#include "square-shape.hpp"

template<typename T, typename A>

__hack_noinline void
run(::utilz::square_shape<T, A>& m)
{
  using size_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;

  const auto x = m.size();
  for (auto k = size_type(0); k < x; ++k)
#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(m) firstprivate(x, k)
#endif
    for (auto i = size_type(0); i < x; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        m.at(i, j) = (std::min)(m.at(i, j), m.at(i, k) + m.at(k, j));
};

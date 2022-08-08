#pragma once

#include "portables/hacks/defines.h"

#include "square-shape.hpp"

template<typename T, typename A>

__hack_noinline void
run(::utilz::square_shape<T, A>& m)
{
  using size_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;

  for (size_type k = size_type(0); k < m.size(); ++k)
#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(m) firstprivate(k)
#endif
    for (size_type i = size_type(0); i < m.size(); ++i)
      for (size_type j = size_type(0); j < m.size(); ++j)
        m.at(i, j) = (std::min)(m.at(i, j), m.at(i, k) + m.at(k, j));
};

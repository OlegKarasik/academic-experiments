#pragma once

#include "square-shape.hpp"

#ifndef _OPENMP
static_assert(false, "erro: selected algorithm requires openMP support, please select appropriate compile options");
#endif

template<typename T>
__attribute__((noinline)) void
calculate_apsp(utilz::square_shape<T>& m)
{
  using size_type = typename utilz::traits::square_shape_traits<utilz::square_shape<T>>::size_type;

  for (size_type k = size_type(0); k < m.size(); ++k)
#pragma omp parallel for
    for (size_type i = size_type(0); i < m.size(); ++i)
      for (size_type j = size_type(0); j < m.size(); ++j)
        m.at(i, j) = std::min(m.at(i, j), m.at(i, k) + m.at(k, j));
};

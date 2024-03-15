#pragma once

#define APSP_ALG_MATRIX

#include "portables/hacks/defines.h"

#include "matrix.hpp"

template<typename T, typename A>

__hack_noinline void
run(::utilz::square_matrix<T, A>& m)
{
  using size_type = typename ::utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;

  const auto x = m.size();
  for (auto k = size_type(0); k < x; ++k)
#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(m) firstprivate(x, k)
#endif
    for (auto i = size_type(0); i < x; ++i)
#ifdef _OPENMP
 #pragma omp simd
#else
      __hack_ivdep
#endif
      for (auto j = size_type(0); j < x; ++j)
        m.at(i, j) = (std::min)(m.at(i, j), m.at(i, k) + m.at(k, j));
};

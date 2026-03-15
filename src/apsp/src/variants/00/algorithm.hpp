#pragma once

#define APSP_ALG_MATRIX_FLAT

#define APSP_ALG_ACCESS_FLAT

#include "portables/hacks/defines.h"

#include "matrix.hpp"
#include "matrix-access.hpp"

namespace utzmx = ::utilz::matrices;

using matrix_type            = utzmx::square_matrix<g_type, g_allocator_type<g_type>>;
using matrix_access_type     = utzmx::access::matrix_access<utzmx::access::matrix_access_schema_flat, matrix_type>;
using matrix_params_type     = utzmx::access::matrix_params<matrix_type>;

__hack_noinline
void
run(
  matrix_type& matrix)
{
  using size_type = typename utzmx::traits::matrix_traits<matrix_type>::size_type;

  const auto x = matrix.size();
  for (auto k = size_type(0); k < x; ++k)
#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(matrix) firstprivate(x, k)
#endif
    for (auto i = size_type(0); i < x; ++i)
#ifdef _OPENMP
  #pragma omp simd
#else
      __hack_ivdep
#endif
      for (auto j = size_type(0); j < x; ++j)
        matrix.at(i, j) = (std::min)(matrix.at(i, j), matrix.at(i, k) + matrix.at(k, j));
};

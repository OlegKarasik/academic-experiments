#pragma once

#define APSP_ALG_MATRIX_BLOCKS

#define APSP_ALG_ACCESS_BLOCKS

#include "portables/hacks/defines.h"

#include "measure.hpp"

#include "matrix.hpp"
#include "matrix-access.hpp"

namespace utzmx = ::utilz::matrices;

using matrix_block_type      = utzmx::square_matrix<g_type, g_allocator_type<g_type>>;
using matrix_type            = utzmx::square_matrix<matrix_block_type, g_allocator_type<matrix_block_type>>;
using matrix_access_type     = utzmx::access::matrix_access<utzmx::access::matrix_access_schema_flat, matrix_type>;
using matrix_params_type     = utzmx::access::matrix_params<matrix_type>;

void
calculate_block(
  matrix_block_type& ij,
  matrix_block_type& ik,
  matrix_block_type& kj)
{
  using size_type = typename utzmx::traits::matrix_traits<matrix_block_type>::size_type;

  const auto x = ij.size();
  for (auto k = size_type(0); k < x; ++k)
    for (auto i = size_type(0); i < x; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

__hack_noinline
void
run(
  matrix_type& matrix)
{
  using size_type = typename utzmx::traits::matrix_traits<matrix_type>::size_type;

#ifdef _OPENMP
  #pragma omp parallel default(none) shared(matrix)
#endif
  {
#ifdef _OPENMP
  #pragma omp single
#endif
    {
      for (auto m = size_type(0); m < matrix.size(); ++m) {
        auto& mm = matrix.at(m, m);

        {
          SCOPE_MEASURE_MILLISECONDS("DIAG");
          calculate_block(mm, mm, mm);
        }

        for (auto i = size_type(0); i < matrix.size(); ++i) {
          if (i != m) {
            auto& im = matrix.at(i, m);
            auto& mi = matrix.at(m, i);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(im, mm)
#endif
            {
              SCOPE_MEASURE_MILLISECONDS("VERT");
              calculate_block(im, im, mm);
            }

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(mi, mm)
#endif
            {
              SCOPE_MEASURE_MILLISECONDS("HORZ");
              calculate_block(mi, mm, mi);
            }
          }
        }
#ifdef _OPENMP
  #pragma omp taskwait
#endif
        for (auto i = size_type(0); i < matrix.size(); ++i) {
          if (i != m) {
            auto& im = matrix.at(i, m);
            for (auto j = size_type(0); j < matrix.size(); ++j) {
              if (j != m) {
                auto& ij = matrix.at(i, j);
                auto& mj = matrix.at(m, j);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(ij, im, mj)
#endif
                {
                  SCOPE_MEASURE_MILLISECONDS("PERH");
                  calculate_block(ij, im, mj);
                }
              }
            }
          }
        }
#ifdef _OPENMP
  #pragma omp taskwait
#endif
      };
    }
  }
};

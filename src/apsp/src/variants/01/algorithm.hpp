#pragma once

#define APSP_ALG_MATRIX_BLOCKS

#include "portables/hacks/defines.h"

#include "measure.hpp"

#include "matrix.hpp"

namespace utzmx = ::utilz::matrices;

template<typename T, typename A>
void
calculate_block(
  utzmx::square_matrix<T, A>& ij,
  utzmx::square_matrix<T, A>& ik,
  utzmx::square_matrix<T, A>& kj)
{
  using size_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::size_type;

  const auto x = ij.size();
  for (auto k = size_type(0); k < x; ++k)
    for (auto i = size_type(0); i < x; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

template<typename T, typename A, typename U>
__hack_noinline
void
run(
  utzmx::square_matrix<utzmx::square_matrix<T, A>, U>& blocks)
{
  using size_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<utzmx::square_matrix<T, A>, U>>::size_type;

#ifdef _OPENMP
  #pragma omp parallel default(none) shared(blocks)
#endif
  {
#ifdef _OPENMP
  #pragma omp single
#endif
    {
      for (auto m = size_type(0); m < blocks.size(); ++m) {
        auto& mm = blocks.at(m, m);

        {
          SCOPE_MEASURE_MILLISECONDS("DIAG");
          calculate_block(mm, mm, mm);
        }

        for (auto i = size_type(0); i < blocks.size(); ++i) {
          if (i != m) {
            auto& im = blocks.at(i, m);
            auto& mi = blocks.at(m, i);

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
        for (auto i = size_type(0); i < blocks.size(); ++i) {
          if (i != m) {
            auto& im = blocks.at(i, m);
            for (auto j = size_type(0); j < blocks.size(); ++j) {
              if (j != m) {
                auto& ij = blocks.at(i, j);
                auto& mj = blocks.at(m, j);

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

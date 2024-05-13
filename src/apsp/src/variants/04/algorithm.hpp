#pragma once

#define APSP_ALG_MATRIX_CLUSTERS

#define APSP_ALG_EXTRA_REARRANGEMENTS

#include "portables/hacks/defines.h"

#include "matrix.hpp"

template<typename T, typename A>
void
calculate_block(
  utilz::matrices::rect_matrix<T, A>& ij,
  utilz::matrices::rect_matrix<T, A>& ik,
  utilz::matrices::rect_matrix<T, A>& kj,
  auto bridges)
{
  using size_type = typename utilz::matrices::traits::matrix_traits<utilz::matrices::rect_matrix<T>>::size_type;

  const auto ij_w = ij.width();
  const auto ij_h = ij.height();

  for (auto k : bridges)
    for (auto i = size_type(0); i < ij_h; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < ij_w; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

template<typename T, typename A>
void
calculate_block(utilz::matrices::rect_matrix<T, A>& ij, utilz::matrices::rect_matrix<T, A>& ik, utilz::matrices::rect_matrix<T, A>& kj)
{
  using size_type = typename utilz::matrices::traits::matrix_traits<utilz::matrices::rect_matrix<T>>::size_type;

  const auto kj_size = kj.height();
  const auto ij_w = ij.width();
  const auto ij_h = ij.height();

  for (auto k = size_type(0); k < kj_size; ++k)
    for (auto i = size_type(0); i < ij_h; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < ij_w; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

template<typename T, typename A, typename U>
__hack_noinline void
run(
  utilz::matrices::square_matrix<utilz::matrices::rect_matrix<T, A>, U>& blocks,
  utilz::matrices::clusters<typename utilz::matrices::traits::matrix_traits<utilz::matrices::rect_matrix<T>>::size_type>& clusters)
{
  using size_type = typename utilz::matrices::traits::matrix_traits<utilz::matrices::square_matrix<utilz::matrices::square_matrix<T, A>, U>>::size_type;

#ifdef _OPENMP
  #pragma omp parallel default(none) shared(blocks, clusters)
#endif
  {
#ifdef _OPENMP
  #pragma omp single
#endif
    {
      for (auto m = size_type(0); m < blocks.size(); ++m) {
        auto& mm = blocks.at(m, m);

        calculate_block(mm, mm, mm);

        auto indeces = clusters.get_indeces(m);

        for (auto i = size_type(0); i < blocks.size(); ++i) {
          if (i != m) {
            auto& im = blocks.at(i, m);
            auto& mi = blocks.at(m, i);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(im, mm, indeces)
#endif
            calculate_block(im, im, mm, indeces);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(mi, mm, indeces)
#endif
            calculate_block(mi, mm, mi, indeces);
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
  #pragma omp task untied default(none) shared(ij, im, mj, indeces)
#endif
                calculate_block(ij, im, mj, indeces);
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

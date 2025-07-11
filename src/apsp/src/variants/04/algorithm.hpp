#pragma once

#define APSP_ALG_MATRIX_CLUSTERS

#define APSP_ALG_EXTRA_CLUSTERS_REARRANGEMENTS

#include "portables/hacks/defines.h"

#include "matrix.hpp"
#include "matrix-traits.hpp"

namespace utzmx = ::utilz::matrices;

template<typename T, typename A>
void
calculate_block(
  utzmx::rect_matrix<T, A>& ij,
  utzmx::rect_matrix<T, A>& ik,
  utzmx::rect_matrix<T, A>& kj,
  auto bridges)
{
  using size_type = typename utzmx::traits::matrix_traits<utzmx::rect_matrix<T>>::size_type;

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
calculate_block(
  utzmx::rect_matrix<T, A>& ij,
  utzmx::rect_matrix<T, A>& ik,
  utzmx::rect_matrix<T, A>& kj)
{
  using size_type = typename utzmx::traits::matrix_traits<utzmx::rect_matrix<T>>::size_type;

  const auto kj_size = kj.height();
  const auto ij_w    = ij.width();
  const auto ij_h    = ij.height();

  for (auto k = size_type(0); k < kj_size; ++k)
    for (auto i = size_type(0); i < ij_h; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < ij_w; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

template<typename T, typename A, typename U>
__hack_noinline
void
run(
  utzmx::square_matrix<utzmx::rect_matrix<T, A>, U>& blocks,
  utzmx::clusters& clusters)
{
  using size_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<utzmx::square_matrix<T, A>, U>>::size_type;

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

        auto positions = clusters.get_all_bridges_positions(m);

        for (auto i = size_type(0); i < blocks.size(); ++i) {
          if (i != m) {
            auto& im = blocks.at(i, m);
            auto& mi = blocks.at(m, i);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(im, mm, positions)
#endif
            calculate_block(im, im, mm, positions);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(mi, mm, positions)
#endif
            calculate_block(mi, mm, mi, positions);
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
  #pragma omp task untied default(none) shared(ij, im, mj, positions)
#endif
                calculate_block(ij, im, mj, positions);
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

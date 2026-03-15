#pragma once

#define APSP_ALG_MATRIX_CLUSTERS
#define APSP_ALG_MATRIX_CLUSTERS_REARRANGEMENTS

#define APSP_ALG_ACCESS_CLUSTERS

#include "portables/hacks/defines.h"

#include "matrix.hpp"
#include "matrix-traits.hpp"
#include "matrix-access.hpp"

namespace utzmx = ::utilz::matrices;

using matrix_clusters_type   = utzmx::clusters;
using matrix_block_type      = utzmx::rect_matrix<g_type, g_allocator_type<g_type>>;
using matrix_type            = utzmx::square_matrix<matrix_block_type, g_allocator_type<matrix_block_type>>;
using matrix_access_type     = utzmx::access::matrix_access<utzmx::access::matrix_access_schema_flat, matrix_type>;
using matrix_params_type     = utzmx::access::matrix_params<matrix_type>;

void
calculate_block(
  matrix_block_type& ij,
  matrix_block_type& ik,
  matrix_block_type& kj,
  auto bridges)
{
  using size_type = typename utzmx::traits::matrix_traits<matrix_block_type>::size_type;

  const auto ij_w = ij.width();
  const auto ij_h = ij.height();

  for (auto k : bridges)
    for (auto i = size_type(0); i < ij_h; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < ij_w; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

void
calculate_block(
  matrix_block_type& ij,
  matrix_block_type& ik,
  matrix_block_type& kj)
{
  using size_type = typename utzmx::traits::matrix_traits<matrix_block_type>::size_type;

  const auto kj_size = kj.height();
  const auto ij_w    = ij.width();
  const auto ij_h    = ij.height();

  for (auto k = size_type(0); k < kj_size; ++k)
    for (auto i = size_type(0); i < ij_h; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < ij_w; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

__hack_noinline
void
run(
  matrix_type& matrix,
  matrix_clusters_type& matrix_clusters)
{
  using size_type = typename utzmx::traits::matrix_traits<matrix_type>::size_type;

#ifdef _OPENMP
  #pragma omp parallel default(none) shared(matrix, matrix_clusters)
#endif
  {
#ifdef _OPENMP
  #pragma omp single
#endif
    {
      for (auto m = size_type(0); m < matrix.size(); ++m) {
        auto& mm = matrix.at(m, m);

        calculate_block(mm, mm, mm);

        auto positions = matrix_clusters.get_all_bridges_positions(m);

        for (auto i = size_type(0); i < matrix.size(); ++i) {
          if (i != m) {
            auto& im = matrix.at(i, m);
            auto& mi = matrix.at(m, i);

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
        for (auto i = size_type(0); i < matrix.size(); ++i) {
          if (i != m) {
            auto& im = matrix.at(i, m);
            for (auto j = size_type(0); j < matrix.size(); ++j) {
              if (j != m) {
                auto& ij = matrix.at(i, j);
                auto& mj = matrix.at(m, j);

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

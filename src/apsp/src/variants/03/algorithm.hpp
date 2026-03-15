#pragma once

#define APSP_ALG_MATRIX_BLOCKS

#define APSP_ALG_ACCESS_BLOCKS

#define APSP_ALG_RUN_CONFIGURATION

#include "portables/hacks/defines.h"

#include <thread>
#include <omp.h>

#include "constants.hpp"
#include "memory.hpp"

#include "matrix.hpp"
#include "matrix-access.hpp"

namespace utzmx = ::utilz::matrices;

template<typename S>
struct run_configuration;

using matrix_block_type      = utzmx::square_matrix<g_type, g_allocator_type<g_type>>;
using matrix_type            = utzmx::square_matrix<matrix_block_type, g_allocator_type<matrix_block_type>>;
using matrix_access_type     = utzmx::access::matrix_access<utzmx::access::matrix_access_schema_flat, matrix_type>;
using matrix_params_type     = utzmx::access::matrix_params<matrix_type>;
using matrix_run_config_type = run_configuration<matrix_type>;

template<typename S>
struct run_configuration
{
  using pointer = typename utzmx::traits::matrix_traits<S>::pointer;

  pointer mm_array_cur_row;
  pointer mm_array_prv_col;
  pointer mm_array_cur_col;
  pointer mm_array_nxt_row;
  pointer ckb1;
  pointer ck1b1;
  pointer ckb1w;
  pointer ckb3w;

  size_t allocation_line;
  size_t allocation_size;
};

void
calculate_diagonal(
  matrix_block_type& mm,
  matrix_run_config_type& run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<matrix_block_type>::size_type;
  using value_type = typename utzmx::traits::matrix_traits<matrix_block_type>::value_type;

  run_config.mm_array_prv_col[0] = ::utilz::constants::infinity<value_type>();
  run_config.mm_array_nxt_row[0] = mm.at(0, 1);

  for (auto k = size_type(1); k < mm.size(); ++k) {
    for (auto i = size_type(0); i < k; ++i)
      run_config.mm_array_cur_row[i] = ::utilz::constants::infinity<value_type>();

    for (auto i = size_type(0); i < k; ++i) {
      const auto x = mm.at(k, i);
      const auto z = run_config.mm_array_prv_col[i];

      auto minimum = ::utilz::constants::infinity<value_type>();

      __hack_ivdep
      for (auto j = size_type(0); j < k; ++j) {
        mm.at(i, j) = (std::min)(mm.at(i, j), z + mm.at(k - 1, j));

        minimum = (std::min)(minimum, mm.at(i, j) + run_config.mm_array_nxt_row[j]);
        run_config.mm_array_cur_row[j] = (std::min)(run_config.mm_array_cur_row[j], mm.at(i, j) + x);
      }
      run_config.mm_array_cur_col[i] = minimum;
    }

    for (auto i = size_type(0); i < k; ++i) {
      mm.at(k, i) = run_config.mm_array_cur_row[i];
      mm.at(i, k) = run_config.mm_array_cur_col[i];

      run_config.mm_array_prv_col[i] = run_config.mm_array_cur_col[i];
      run_config.mm_array_nxt_row[i] = mm.at(i, k + 1);
    }

    if (k < (mm.size() - 1))
      run_config.mm_array_nxt_row[k] = mm.at(k, k + 1);
  }

  const auto x = mm.size() - size_type(1);
  for (auto i = size_type(0); i < x; ++i) {
    const auto ix = run_config.mm_array_prv_col[i];

    __hack_ivdep
    for (auto j = size_type(0); j < x; ++j)
      mm.at(i, j) = (std::min)(mm.at(i, j), ix + mm.at(x, j));
  }
}

void
calculate_vertical(
  matrix_block_type& im,
  matrix_block_type& mm,
  matrix_run_config_type& run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<matrix_block_type>::size_type;
  using value_type = typename utzmx::traits::matrix_traits<matrix_block_type>::value_type;
  using pointer    = typename utzmx::traits::matrix_traits<matrix_block_type>::pointer;

#ifdef _OPENMP
  auto allocation_shift = run_config.allocation_line * omp_get_thread_num();
#else
  auto allocation_shift = 0;
#endif

  pointer im_array_prv_weight = run_config.mm_array_prv_col + allocation_shift;
  pointer im_array_cur_weight = run_config.mm_array_cur_row + allocation_shift;
  pointer mm_array_nxt_weight = run_config.ckb3w + allocation_shift;

  const auto x = im.size();

  __hack_ivdep
  for (auto i = size_type(0); i < x; ++i) {
    im_array_prv_weight[i] = ::utilz::constants::infinity<value_type>();
    mm_array_nxt_weight[i] = mm.at(i, 1);
  }
  for (auto k = size_type(1); k < x; ++k) {
    const auto z = k - size_type(1);
    for (auto i = size_type(0); i < x; ++i) {
      const auto v = im_array_prv_weight[i];

      auto minimum = im.at(i, k);

      __hack_ivdep
      for (auto j = size_type(0); j < k; ++j) {
        im.at(i, j) = (std::min)(im.at(i, j), v + mm.at(z, j));

        minimum = (std::min)(minimum, im.at(i, j) + mm_array_nxt_weight[j]);
      }
      im_array_cur_weight[i] = minimum;
    }
    for (auto i = size_type(0); i < im.size(); ++i) {
      im.at(i, k) = im_array_cur_weight[i];

      mm_array_nxt_weight[i] = mm.at(i, k + 1);
    }
    std::swap(im_array_prv_weight, im_array_cur_weight);
  }

  const auto z = x - size_type(1);
  for (auto i = size_type(0); i < x; ++i) {
    const auto v = im_array_prv_weight[i];

    __hack_ivdep
    for (auto j = size_type(0); j < z; ++j)
      im.at(i, j) = (std::min)(im.at(i, j), v + mm.at(z, j));
  }
}

void
calculate_horizontal(
  matrix_block_type& mi,
  matrix_block_type& mm,
  matrix_run_config_type& run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<matrix_block_type>::size_type;
  using pointer    = typename utzmx::traits::matrix_traits<matrix_block_type>::pointer;

#ifdef _OPENMP
  auto allocation_shift = run_config.allocation_line * omp_get_thread_num();
#else
  auto allocation_shift = 0;
#endif

  pointer mm_array_nxt_weight = run_config.ckb1 + allocation_shift;

  const auto x = mi.size();

  mm_array_nxt_weight[0] = mm.at(0, 0);
  for (auto k = size_type(1); k < x; ++k) {
    auto const z = k - size_type(1);
    for (auto i = size_type(0); i < k; ++i) {
      const auto v = mm_array_nxt_weight[i];
      const auto w = mm.at(k, i);

      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j) {
        mi.at(i, j) = (std::min)(mi.at(i, j), v + mi.at(z, j));
        mi.at(k, j) = (std::min)(mi.at(k, j), w + mi.at(i, j));
      }
    }

    for (auto i = size_type(0); i < k; ++i)
      mm_array_nxt_weight[i] = mm.at(i, k);
  }

  const auto z = x - size_type(1);
  for (auto i = size_type(0); i < z; ++i) {
    const auto v = mm_array_nxt_weight[i];

    __hack_ivdep
    for (auto j = size_type(0); j < x; ++j)
      mi.at(i, j) = (std::min)(mi.at(i, j), v + mi.at(z, j));
  }
}

void
calculate_peripheral(
  matrix_block_type& ij,
  matrix_block_type& ik,
  matrix_block_type& kj)
{
  using size_type = typename utzmx::traits::matrix_traits<matrix_block_type>::size_type;

  const auto x = ij.size();
  for (auto i = size_type(0); i < x; ++i)
    for (auto k = size_type(0); k < x; ++k)
      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
}


__hack_noinline
void
up(
  matrix_type&             matrix,
  matrix_access_type&      matrix_access,
  matrix_run_config_type&  matrix_run_config,
  ::utilz::memory::buffer& b)
{
  using size_type  = typename utzmx::traits::matrix_traits<matrix_type>::size_type;
  using value_type = typename utzmx::traits::matrix_traits<matrix_type>::value_type;
  using pointer    = typename utzmx::traits::matrix_traits<matrix_type>::pointer;

#ifdef _OPENMP
  auto allocation_mulx = std::thread::hardware_concurrency();
#else
  auto allocation_mulx = 1;
#endif

  auto allocation_line = matrix_access.dimensions().max();
  auto allocation_size = allocation_line * sizeof(value_type) * allocation_mulx;

  matrix_run_config.allocation_line = allocation_line;
  matrix_run_config.allocation_size = allocation_size;
  matrix_run_config.mm_array_cur_row = reinterpret_cast<pointer>(b.allocate(allocation_size));
  matrix_run_config.mm_array_prv_col = reinterpret_cast<pointer>(b.allocate(allocation_size));
  matrix_run_config.mm_array_cur_col = reinterpret_cast<pointer>(b.allocate(allocation_size));
  matrix_run_config.mm_array_nxt_row = reinterpret_cast<pointer>(b.allocate(allocation_size));
  matrix_run_config.ckb1  = reinterpret_cast<pointer>(b.allocate(allocation_size));
  matrix_run_config.ck1b1 = reinterpret_cast<pointer>(b.allocate(allocation_size));
  matrix_run_config.ckb1w = reinterpret_cast<pointer>(b.allocate(allocation_size));
  matrix_run_config.ckb3w = reinterpret_cast<pointer>(b.allocate(allocation_size));

  for (size_type i = size_type(0); i < allocation_line * allocation_mulx; ++i) {
    matrix_run_config.mm_array_cur_row[i] = ::utilz::constants::infinity<value_type>();
    matrix_run_config.mm_array_prv_col[i] = ::utilz::constants::infinity<value_type>();
    matrix_run_config.mm_array_cur_col[i] = ::utilz::constants::infinity<value_type>();
    matrix_run_config.mm_array_nxt_row[i] = ::utilz::constants::infinity<value_type>();
    matrix_run_config.ckb1[i]  = ::utilz::constants::infinity<value_type>();
    matrix_run_config.ck1b1[i] = ::utilz::constants::infinity<value_type>();
    matrix_run_config.ckb1w[i] = ::utilz::constants::infinity<value_type>();
    matrix_run_config.ckb3w[i] = ::utilz::constants::infinity<value_type>();
  }
};

__hack_noinline
void
down(
  matrix_type&             matrix,
  matrix_access_type&      matrix_access,
  matrix_run_config_type&  matrix_run_config,
  ::utilz::memory::buffer& b)
{
  using alptr_type = typename ::utilz::memory::buffer::pointer;

  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.mm_array_cur_row), matrix_run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.mm_array_prv_col), matrix_run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.mm_array_cur_col), matrix_run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.mm_array_nxt_row), matrix_run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.ckb1),  matrix_run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.ck1b1), matrix_run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.ckb1w), matrix_run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.ckb3w), matrix_run_config.allocation_size);
}

__hack_noinline
void
run(
  matrix_type&            matrix,
  matrix_run_config_type& matrix_run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<matrix_type>::size_type;
#ifdef _OPENMP
  #pragma omp parallel default(none) shared(matrix, matrix_run_config)
#endif
  {
#ifdef _OPENMP
  #pragma omp single
#endif
    {
      for (auto m = size_type(0); m < matrix.size(); ++m) {
        auto& mm = matrix.at(m, m);

        calculate_diagonal(mm, matrix_run_config);
        for (auto i = size_type(0); i < matrix.size(); ++i) {
          if (i != m) {
            auto& im = matrix.at(i, m);
            auto& mi = matrix.at(m, i);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(im, mm, matrix_run_config)
#endif
            calculate_vertical(im, mm, matrix_run_config);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(mi, mm, matrix_run_config)
#endif
            calculate_horizontal(mi, mm, matrix_run_config);
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
                calculate_peripheral(ij, im, mj);
              }
            }
          }
        }
#ifdef _OPENMP
  #pragma omp taskwait
#endif
      }
    }
  }
};

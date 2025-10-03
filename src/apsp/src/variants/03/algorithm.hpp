#pragma once

#define APSP_ALG_MATRIX_BLOCKS

#define APSP_ALG_EXTRA_CONFIGURATION

#include "portables/hacks/defines.h"

#include "memory.hpp"
#include "matrix.hpp"
#include "constants.hpp"

#include <thread>
#include <omp.h>

namespace utzmx = ::utilz::matrices;

template<typename T, typename A, typename U>
struct run_configuration
{
  using pointer = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T>>::pointer;

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

template<typename T, typename A, typename U>
void
calculate_diagonal(
  utzmx::square_matrix<T, A>& mm,
  run_configuration<T, A, U>& run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::size_type;
  using value_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::value_type;

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

template<typename T, typename A, typename U>
void
calculate_vertical(
  utzmx::square_matrix<T, A>& im,
  utzmx::square_matrix<T, A>& mm,
  run_configuration<T, A, U>& run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::size_type;
  using value_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::value_type;
  using pointer    = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::pointer;

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

template<typename T, typename A, typename U>
void
calculate_horizontal(
  utzmx::square_matrix<T, A>& mi,
  utzmx::square_matrix<T, A>& mm,
  run_configuration<T, A, U>& run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::size_type;
  using pointer    = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::pointer;

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

template<typename T, typename A>
void
calculate_peripheral(
  utzmx::square_matrix<T, A>& ij,
  utzmx::square_matrix<T, A>& ik,
  utzmx::square_matrix<T, A>& kj)
{
  using size_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::size_type;

  const auto x = ij.size();
  for (auto i = size_type(0); i < x; ++i)
    for (auto k = size_type(0); k < x; ++k)
      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
}


template<typename T, typename A, typename U>
__hack_noinline
void
up(
  utzmx::matrix_abstract<utzmx::square_matrix<utzmx::square_matrix<T, A>, U>>& abstract,
  utilz::memory::buffer& b,
  run_configuration<T, A, U>& run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<utzmx::square_matrix<utzmx::square_matrix<T, A>, U>>::size_type;
  using value_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<utzmx::square_matrix<T, A>, U>>::value_type;

#ifdef _OPENMP
  auto allocation_mulx = std::thread::hardware_concurrency();
#else
  auto allocation_mulx = 1;
#endif

  auto allocation_line = abstract.size();
  auto allocation_size = allocation_line * sizeof(value_type) * allocation_mulx;

  run_config.allocation_line = allocation_line;
  run_config.allocation_size = allocation_size;
  run_config.mm_array_cur_row = reinterpret_cast<T*>(b.allocate(allocation_size));
  run_config.mm_array_prv_col = reinterpret_cast<T*>(b.allocate(allocation_size));
  run_config.mm_array_cur_col = reinterpret_cast<T*>(b.allocate(allocation_size));
  run_config.mm_array_nxt_row = reinterpret_cast<T*>(b.allocate(allocation_size));
  run_config.ckb1  = reinterpret_cast<T*>(b.allocate(allocation_size));
  run_config.ck1b1 = reinterpret_cast<T*>(b.allocate(allocation_size));
  run_config.ckb1w = reinterpret_cast<T*>(b.allocate(allocation_size));
  run_config.ckb3w = reinterpret_cast<T*>(b.allocate(allocation_size));

  for (size_type i = size_type(0); i < allocation_line * allocation_mulx; ++i) {
    run_config.mm_array_cur_row[i] = ::utilz::constants::infinity<value_type>();
    run_config.mm_array_prv_col[i] = ::utilz::constants::infinity<value_type>();
    run_config.mm_array_cur_col[i] = ::utilz::constants::infinity<value_type>();
    run_config.mm_array_nxt_row[i] = ::utilz::constants::infinity<value_type>();
    run_config.ckb1[i]  = ::utilz::constants::infinity<value_type>();
    run_config.ck1b1[i] = ::utilz::constants::infinity<value_type>();
    run_config.ckb1w[i] = ::utilz::constants::infinity<value_type>();
    run_config.ckb3w[i] = ::utilz::constants::infinity<value_type>();
  }
};

template<typename T, typename A, typename U>
__hack_noinline
void
down(
  utzmx::matrix_abstract<utzmx::square_matrix<utzmx::square_matrix<T, A>, U>>& abstract,
  ::utilz::memory::buffer& b,
  run_configuration<T, A, U>& run_config)
{
  using alptr_type = typename ::utilz::memory::buffer::pointer;

  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_cur_row), run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_prv_col), run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_cur_col), run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_nxt_row), run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.ckb1), run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.ck1b1), run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.ckb1w), run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.ckb3w), run_config.allocation_size);
}

template<typename T, typename A, typename U>
__hack_noinline
void
run(
  utzmx::square_matrix<utzmx::square_matrix<T, A>, U>& blocks,
  run_configuration<T, A, U>& run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<utzmx::square_matrix<utzmx::square_matrix<T, A>, U>>::size_type;
#ifdef _OPENMP
  #pragma omp parallel default(none) shared(blocks, run_config)
#endif
  {
#ifdef _OPENMP
  #pragma omp single
#endif
    {
      for (auto m = size_type(0); m < blocks.size(); ++m) {
        auto& mm = blocks.at(m, m);

        calculate_diagonal(mm, run_config);
        for (auto i = size_type(0); i < blocks.size(); ++i) {
          if (i != m) {
            auto& im = blocks.at(i, m);
            auto& mi = blocks.at(m, i);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(im, mm, run_config)
#endif
            calculate_vertical(im, mm, run_config);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(mi, mm, run_config)
#endif
            calculate_horizontal(mi, mm, run_config);
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

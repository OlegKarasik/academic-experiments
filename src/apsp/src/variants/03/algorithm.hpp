#pragma once

#define APSP_ALG_HAS_BLOCKS
#define APSP_ALG_HAS_OPTIONS

#include "portables/hacks/defines.h"

#include "memory.hpp"
#include "square-shape.hpp"
#include "constants.hpp"

#include <thread>
#include <omp.h>

template<typename T>
struct support_arrays
{
  using pointer = typename utilz::traits::matrix_traits<utilz::square_matrix<T>>::pointer;

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
__hack_noinline support_arrays<T>
up(utilz::square_matrix<utilz::square_matrix<T, A>, U>& blocks, utilz::memory::buffer& b)
{
  using pointer    = typename utilz::traits::matrix_traits<utilz::square_matrix<utilz::square_matrix<T, A>, U>>::pointer;
  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<utilz::square_matrix<T, A>, U>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<utilz::square_matrix<T, A>, U>>::value_type;

  utilz::procedures::matrix_get_dimensions<utilz::square_matrix<utilz::square_matrix<T, A>, U>> sz;
  utilz::procedures::matrix_at<utilz::square_matrix<utilz::square_matrix<T, A>, U>> at;

#ifdef _OPENMP
  auto allocation_mulx = std::thread::hardware_concurrency();
#else
  auto allocation_mulx = 1;
#endif

  auto allocation_line = sz(blocks).s();
  auto allocation_size = allocation_line * sizeof(value_type) * allocation_mulx;

  support_arrays<T> arrays;

  arrays.allocation_line = allocation_line;
  arrays.allocation_size = allocation_size;
  arrays.mm_array_cur_row = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.mm_array_prv_col = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.mm_array_cur_col = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.mm_array_nxt_row = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.ckb1 = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.ck1b1 = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.ckb1w = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.ckb3w = reinterpret_cast<T*>(b.allocate(allocation_size));

  // The algorithm requires that all self-loops have non "infinite" value. This
  // doesn't affect correctness of calculations.
  //
  for (size_type i = size_type(0); i < sz(blocks).s(); ++i) {
    if (at(blocks, i, i) == utilz::constants::infinity<value_type>())
      at(blocks, i, i) = size_type(0);
  }

  for (size_type i = size_type(0); i < allocation_line * allocation_mulx; ++i) {
    arrays.mm_array_cur_row[i] = utilz::constants::infinity<value_type>();
    arrays.mm_array_prv_col[i] = utilz::constants::infinity<value_type>();
    arrays.mm_array_cur_col[i] = utilz::constants::infinity<value_type>();
    arrays.mm_array_nxt_row[i] = utilz::constants::infinity<value_type>();
    arrays.ckb1[i]  = utilz::constants::infinity<value_type>();
    arrays.ck1b1[i] = utilz::constants::infinity<value_type>();
    arrays.ckb1w[i] = utilz::constants::infinity<value_type>();
    arrays.ckb3w[i] = utilz::constants::infinity<value_type>();
  }

  return arrays;
};

template<typename T, typename A, typename U>
__hack_noinline void
down(utilz::square_matrix<utilz::square_matrix<T, A>, U>& blocks, utilz::memory::buffer& b, support_arrays<T> o)
{
  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<utilz::square_matrix<T, A>, U>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<utilz::square_matrix<T, A>, U>>::value_type;

  using alptr_type = typename utilz::memory::buffer::pointer;

  utilz::procedures::matrix_get_dimensions<utilz::square_matrix<utilz::square_matrix<T, A>, U>> sz;
  utilz::procedures::matrix_at<utilz::square_matrix<utilz::square_matrix<T, A>, U>> at;

  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_cur_row), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_prv_col), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_cur_col), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_nxt_row), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.ckb1), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.ck1b1), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.ckb1w), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.ckb3w), o.allocation_size);

  // Restoring the matrix to a state where self-loop is represented as
  // infinity instead of 0.
  //
  for (size_type i = size_type(0); i < sz(blocks).s(); ++i)
    if (at(blocks, i, i) == size_type(0))
      at(blocks, i, i) = utilz::constants::infinity<value_type>();
}

template<typename T, typename A>
void
calculate_diagonal(utilz::square_matrix<T, A>& mm, support_arrays<T>& support_arrays)
{
  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::value_type;

  support_arrays.mm_array_prv_col[0] = utilz::constants::infinity<value_type>();
  support_arrays.mm_array_nxt_row[0] = mm.at(0, 1);

  for (auto k = size_type(1); k < mm.size(); ++k) {
    for (auto i = size_type(0); i < k; ++i)
      support_arrays.mm_array_cur_row[i] = utilz::constants::infinity<value_type>();

    for (auto i = size_type(0); i < k; ++i) {
      const auto x = mm.at(k, i);
      const auto z = support_arrays.mm_array_prv_col[i];

      auto minimum = utilz::constants::infinity<value_type>();

      __hack_ivdep
      for (auto j = size_type(0); j < k; ++j) {
        mm.at(i, j) = (std::min)(mm.at(i, j), z + mm.at(k - 1, j));

        minimum = (std::min)(minimum, mm.at(i, j) + support_arrays.mm_array_nxt_row[j]);
        support_arrays.mm_array_cur_row[j] = (std::min)(support_arrays.mm_array_cur_row[j], mm.at(i, j) + x);
      }
      support_arrays.mm_array_cur_col[i] = minimum;
    }

    for (auto i = size_type(0); i < k; ++i) {
      mm.at(k, i) = support_arrays.mm_array_cur_row[i];
      mm.at(i, k) = support_arrays.mm_array_cur_col[i];

      support_arrays.mm_array_prv_col[i] = support_arrays.mm_array_cur_col[i];
      support_arrays.mm_array_nxt_row[i] = mm.at(i, k + 1);
    }

    if (k < (mm.size() - 1))
      support_arrays.mm_array_nxt_row[k] = mm.at(k, k + 1);
  }

  const auto x = mm.size() - size_type(1);
  for (auto i = size_type(0); i < x; ++i) {
    const auto ix = support_arrays.mm_array_prv_col[i];

    __hack_ivdep
    for (auto j = size_type(0); j < x; ++j)
      mm.at(i, j) = (std::min)(mm.at(i, j), ix + mm.at(x, j));
  }
}

template<typename T, typename A>
void
calculate_horizontal(utilz::square_matrix<T, A>& im, utilz::square_matrix<T, A>& mm, support_arrays<T>& support_arrays)
{
  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::value_type;
  using pointer    = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::pointer;

#ifdef _OPENMP
  auto allocation_shift = support_arrays.allocation_line * omp_get_thread_num();
#else
  auto allocation_shift = 0;
#endif

  pointer im_array_prv_weight = support_arrays.mm_array_prv_col + allocation_shift;
  pointer im_array_cur_weight = support_arrays.mm_array_cur_row + allocation_shift;

  pointer im_array_nxt_weight = support_arrays.ckb1w + allocation_shift;
  pointer mm_array_nxt_weight = support_arrays.ckb3w + allocation_shift;

  const auto x = im.size();

  __hack_ivdep
  for (auto i = size_type(0); i < x; ++i) {
    im_array_prv_weight[i] = utilz::constants::infinity<value_type>();
    im_array_nxt_weight[i] = im.at(i, 1);
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

      im_array_nxt_weight[i] = im.at(i, k + 1);
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

template<typename T, typename A>
void
calculate_vertical(utilz::square_matrix<T, A>& mi, utilz::square_matrix<T, A>& mm, support_arrays<T>& support_arrays)
{
  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::value_type;
  using pointer    = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::pointer;

#ifdef _OPENMP
  auto allocation_shift = support_arrays.allocation_line * omp_get_thread_num();
#else
  auto allocation_shift = 0;
#endif

  pointer mm_array_nxt_weight = support_arrays.ckb1 + allocation_shift;

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
calculate_peripheral(utilz::square_matrix<T, A>& ij, utilz::square_matrix<T, A>& ik, utilz::square_matrix<T, A>& kj)
{
  using size_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T>>::size_type;

  const auto x = ij.size();
  for (auto k = size_type(0); k < x; ++k)
    for (auto i = size_type(0); i < x; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
}

template<typename T, typename A, typename U>
__hack_noinline void
run(utilz::square_matrix<utilz::square_matrix<T, A>, U>& blocks, support_arrays<T>& support_arrays)
{
  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
#ifdef _OPENMP
  #pragma omp parallel default(none) shared(blocks, support_arrays)
#endif
  {
#ifdef _OPENMP
  #pragma omp single
#endif
    {
      for (auto m = size_type(0); m < blocks.size(); ++m) {
        auto& mm = blocks.at(m, m);

        calculate_diagonal(mm, support_arrays);
        for (auto i = size_type(0); i < blocks.size(); ++i) {
          if (i != m) {
            auto& im = blocks.at(i, m);
            auto& mi = blocks.at(m, i);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(im, mm, support_arrays)
#endif
            calculate_horizontal(im, mm, support_arrays);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(mi, mm, support_arrays)
#endif
            calculate_vertical(mi, mm, support_arrays);
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

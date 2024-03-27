#pragma once

#define APSP_ALG_MATRIX

#define APSP_ALG_EXTRA_OPTIONS

#include "portables/hacks/defines.h"

#include "memory.hpp"
#include "matrix.hpp"
#include "constants.hpp"

template<typename T>
struct support_arrays
{
  using pointer = typename ::utilz::traits::matrix_traits<utilz::square_matrix<T>>::pointer;

  pointer mm_array_cur_row;
  pointer mm_array_prv_col;
  pointer mm_array_cur_col;
  pointer mm_array_nxt_col;
};

template<typename T, typename A>
__hack_noinline support_arrays<T>
up(utilz::square_matrix<T, A>& m, utilz::memory::buffer& b)
{
  using pointer    = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::pointer;
  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::value_type;

  auto allocation_size = m.size() * sizeof(value_type);

  support_arrays<T> arrays;

  arrays.mm_array_cur_row = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.mm_array_prv_col = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.mm_array_cur_col = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.mm_array_nxt_col = reinterpret_cast<pointer>(b.allocate(allocation_size));

  for (auto i = size_type(0); i < m.size(); ++i) {
    arrays.mm_array_cur_row[i] = utilz::constants::infinity<value_type>();
    arrays.mm_array_prv_col[i] = utilz::constants::infinity<value_type>();
    arrays.mm_array_cur_col[i] = utilz::constants::infinity<value_type>();
    arrays.mm_array_nxt_col[i] = utilz::constants::infinity<value_type>();
  }

  return arrays;
};

template<typename T, typename A>
__hack_noinline void
down(utilz::square_matrix<T, A>& m, utilz::memory::buffer& b, support_arrays<T> o)
{
  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::value_type;

  using alptr_type = typename utilz::memory::buffer::pointer;

  auto allocation_size = m.size() * sizeof(value_type);

  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_cur_row), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_prv_col), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_cur_col), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_nxt_col), allocation_size);
}

template<typename T, typename A>
__hack_noinline void
run(utilz::square_matrix<T, A>& m, support_arrays<T>& support_arrays)
{
  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::value_type;

  support_arrays.mm_array_prv_col[0] = utilz::constants::infinity<value_type>();
  support_arrays.mm_array_nxt_col[0] = m.at(0, 1);

  const auto x = m.size() - size_type(1);
  for (auto k = size_type(1); k < m.size(); ++k) {
    __hack_ivdep
    for (auto i = size_type(0); i < k; ++i)
      support_arrays.mm_array_cur_row[i] = utilz::constants::infinity<value_type>();

    const auto z = k - size_type(1);

#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(m, support_arrays) firstprivate(k, x, z)
#endif
    for (auto i = size_type(0); i < k; ++i) {
      const auto v = m.at(k, i);
      const auto w = support_arrays.mm_array_prv_col[i];

      auto minimum = utilz::constants::infinity<value_type>();

#ifdef _OPENMP
  #pragma omp simd
#else
      __hack_ivdep
#endif
      for (auto j = size_type(0); j < k; ++j) {
        m.at(i, j) = (std::min)(m.at(i, j), w + m.at(z, j));

        minimum = (std::min)(minimum, m.at(i, j) + support_arrays.mm_array_nxt_col[j]);

        support_arrays.mm_array_cur_row[j] = (std::min)(support_arrays.mm_array_cur_row[j], m.at(i, j) + v);
      }
      support_arrays.mm_array_cur_col[i] = minimum;
    }

#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(m, support_arrays) firstprivate(k)
#endif
    for (auto i = size_type(0); i < k; ++i) {
      m.at(k, i) = support_arrays.mm_array_cur_row[i];
      m.at(i, k) = support_arrays.mm_array_cur_col[i];

      support_arrays.mm_array_prv_col[i] = support_arrays.mm_array_cur_col[i];
      support_arrays.mm_array_nxt_col[i] = m.at(i, k + size_type(1));
    }

    if (k < x)
      support_arrays.mm_array_nxt_col[k] = m.at(k, k + size_type(1));
  }

#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(m, support_arrays) firstprivate(x)
#endif
  for (auto i = size_type(0); i < x; ++i) {
    const auto v = support_arrays.mm_array_prv_col[i];

#ifdef _OPENMP
  #pragma omp simd
#else
    __hack_ivdep
#endif
    for (auto j = size_type(0); j < x; ++j)
      m.at(i, j) = (std::min)(m.at(i, j), v + m.at(x, j));
  }
};

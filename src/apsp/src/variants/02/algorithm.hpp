#pragma once

#define APSP_ALG_HAS_OPTIONS

#include "portables/hacks/defines.h"

#include "memory.hpp"
#include "square-shape.hpp"

#include "constants.hpp"

template<typename T>
struct support_arrays
{
  using pointer = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T>>::pointer;

  pointer mm_array_cur_row;
  pointer mm_array_prv_col;
  pointer mm_array_cur_col;
  pointer mm_array_nxt_col;
};

template<typename T, typename A>
__hack_noinline support_arrays<T>
up(::utilz::square_shape<T, A>& m, ::utilz::memory::buffer& b)
{
  using pointer    = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::pointer;
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;

  auto allocation_size = m.size() * sizeof(value_type);

  support_arrays<T> arrays;

  arrays.mm_array_cur_row = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.mm_array_prv_col = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.mm_array_cur_col = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.mm_array_nxt_col = reinterpret_cast<pointer>(b.allocate(allocation_size));

  // The algorithm requires that all self-loops have non "infinite" value. This
  // doesn't affect correctness of calculations.
  //
  for (auto i = size_type(0); i < m.size(); ++i) {
    if (m.at(i, i) == ::apsp::constants::infinity<value_type>())
      m.at(i, i) = size_type(0);

    arrays.mm_array_cur_row[i] = ::apsp::constants::infinity<value_type>();
    arrays.mm_array_prv_col[i] = ::apsp::constants::infinity<value_type>();
    arrays.mm_array_cur_col[i] = ::apsp::constants::infinity<value_type>();
    arrays.mm_array_nxt_col[i] = ::apsp::constants::infinity<value_type>();
  }

  return arrays;
};

template<typename T, typename A>
__hack_noinline void
down(::utilz::square_shape<T, A>& m, ::utilz::memory::buffer& b, support_arrays<T> o)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;

  using alptr_type = typename ::utilz::memory::buffer::pointer;

  auto allocation_size = m.size() * sizeof(value_type);

  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_cur_row), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_prv_col), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_cur_col), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mm_array_nxt_col), allocation_size);

  // Restoring the matrix to a state where self-loop is represented as
  // infinity instead of 0.
  //
  for (auto i = size_type(0); i < m.size(); ++i)
    if (m.at(i, i) == size_type(0))
      m.at(i, i) = ::apsp::constants::infinity<value_type>();
}

template<typename T, typename A>
__hack_noinline void
run(::utilz::square_shape<T, A>& m, support_arrays<T>& support_arrays)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;

  support_arrays.mm_array_prv_col[0] = ::apsp::constants::infinity<value_type>();
  support_arrays.mm_array_nxt_col[0] = m.at(0, 1);

  const auto x = m.size() - size_type(1);
  for (auto k = size_type(1); k < m.size(); ++k) {
    __hack_ivdep
    for (auto i = size_type(0); i < k; ++i)
      support_arrays.mm_array_cur_row[i] = ::apsp::constants::infinity<value_type>();

    const auto z = k - size_type(1);

#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(m, support_arrays) firstprivate(x) schedule(dynamic, 1)
#endif
    for (auto i = size_type(0); i < k; ++i) {
      const auto v = m.at(k, i);
      const auto w = support_arrays.mm_array_prv_col[i];

      auto minimum = ::apsp::constants::infinity<value_type>();

      __hack_ivdep
      for (auto j = size_type(0); j < k; ++j) {
        m.at(i, j) = (std::min)(m.at(i, j), w + m.at(z, j));

        minimum = (std::min)(minimum, m.at(i, j) + support_arrays.mm_array_nxt_col[j]);

        support_arrays.mm_array_cur_row[j] = (std::min)(support_arrays.mm_array_cur_row[j], m.at(i, j) + v);
      }
      support_arrays.mm_array_cur_col[i] = minimum;
    }

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

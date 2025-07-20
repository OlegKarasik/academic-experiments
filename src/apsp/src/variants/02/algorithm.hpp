#pragma once

#define APSP_ALG_MATRIX

#define APSP_ALG_EXTRA_CONFIGURATION

#include "portables/hacks/defines.h"

#include "memory.hpp"
#include "matrix.hpp"
#include "constants.hpp"

namespace utzmx = ::utilz::matrices;

template<typename T, typename A>
struct run_configuration
{
  using pointer = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::pointer;

  pointer mm_array_cur_row;
  pointer mm_array_prv_col;
  pointer mm_array_cur_col;
  pointer mm_array_nxt_col;
};

template<typename T, typename A>
__hack_noinline
void
up(
  utzmx::matrix_abstract<utzmx::square_matrix<T, A>>& abstract,
  ::utilz::memory::buffer& b,
  run_configuration<T, A>& run_config)
{
  using pointer    = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::pointer;
  using size_type  = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::size_type;
  using value_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::value_type;

  auto allocation_size = abstract.size() * sizeof(value_type);

  run_config.mm_array_cur_row = reinterpret_cast<pointer>(b.allocate(allocation_size));
  run_config.mm_array_prv_col = reinterpret_cast<pointer>(b.allocate(allocation_size));
  run_config.mm_array_cur_col = reinterpret_cast<pointer>(b.allocate(allocation_size));
  run_config.mm_array_nxt_col = reinterpret_cast<pointer>(b.allocate(allocation_size));

  for (auto i = size_type(0); i < abstract.size(); ++i) {
    run_config.mm_array_cur_row[i] = ::utilz::constants::infinity<value_type>();
    run_config.mm_array_prv_col[i] = ::utilz::constants::infinity<value_type>();
    run_config.mm_array_cur_col[i] = ::utilz::constants::infinity<value_type>();
    run_config.mm_array_nxt_col[i] = ::utilz::constants::infinity<value_type>();
  }
};

template<typename T, typename A>
__hack_noinline
void
down(
  utzmx::matrix_abstract<utzmx::square_matrix<T, A>>& abstract,
  ::utilz::memory::buffer& b,
  run_configuration<T, A>& run_config)
{
  using value_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::value_type;

  using alptr_type = typename ::utilz::memory::buffer::pointer;

  auto allocation_size = abstract.size() * sizeof(value_type);

  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_cur_row), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_prv_col), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_cur_col), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_nxt_col), allocation_size);
}

template<typename T, typename A>
__hack_noinline
void
run(
  utzmx::square_matrix<T, A>& m,
  run_configuration<T, A>& run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::size_type;
  using value_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::value_type;

  run_config.mm_array_prv_col[0] = ::utilz::constants::infinity<value_type>();
  run_config.mm_array_nxt_col[0] = m.at(0, 1);

  const auto x = m.size() - size_type(1);
  for (auto k = size_type(1); k < m.size(); ++k) {
    __hack_ivdep
    for (auto i = size_type(0); i < k; ++i)
      run_config.mm_array_cur_row[i] = ::utilz::constants::infinity<value_type>();

    const auto z = k - size_type(1);

    for (auto i = size_type(0); i < k; ++i) {
      const auto v = m.at(k, i);
      const auto w = run_config.mm_array_prv_col[i];

      auto minimum = ::utilz::constants::infinity<value_type>();

      __hack_ivdep
      for (auto j = size_type(0); j < k; ++j) {
        m.at(i, j) = (std::min)(m.at(i, j), w + m.at(z, j));

        minimum = (std::min)(minimum, m.at(i, j) + run_config.mm_array_nxt_col[j]);

        run_config.mm_array_cur_row[j] = (std::min)(run_config.mm_array_cur_row[j], m.at(i, j) + v);
      }
      run_config.mm_array_cur_col[i] = minimum;
    }

#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(m, run_config) firstprivate(k)
#endif
    for (auto i = size_type(0); i < k; ++i) {
      m.at(k, i) = run_config.mm_array_cur_row[i];
      m.at(i, k) = run_config.mm_array_cur_col[i];

      run_config.mm_array_prv_col[i] = run_config.mm_array_cur_col[i];
      run_config.mm_array_nxt_col[i] = m.at(i, k + size_type(1));
    }

    if (k < x)
      run_config.mm_array_nxt_col[k] = m.at(k, k + size_type(1));
  }

#ifdef _OPENMP
  #pragma omp parallel for default(none) shared(m, run_config) firstprivate(x)
#endif
  for (auto i = size_type(0); i < x; ++i) {
    const auto v = run_config.mm_array_prv_col[i];

#ifdef _OPENMP
  #pragma omp simd
#else
    __hack_ivdep
#endif
    for (auto j = size_type(0); j < x; ++j)
      m.at(i, j) = (std::min)(m.at(i, j), v + m.at(x, j));
  }
};

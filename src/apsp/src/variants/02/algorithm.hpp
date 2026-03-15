#pragma once

#define APSP_ALG_MATRIX_FLAT

#define APSP_ALG_ACCESS_FLAT

#define APSP_ALG_RUN_CONFIGURATION

#include "portables/hacks/defines.h"

#include "constants.hpp"
#include "memory.hpp"

#include "matrix.hpp"
#include "matrix-access.hpp"

namespace utzmx = ::utilz::matrices;

template<typename S>
struct run_configuration;

using matrix_type            = utzmx::square_matrix<g_type, g_allocator_type<g_type>>;
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
  pointer mm_array_nxt_col;
};

__hack_noinline
void
up(
  matrix_type&        matrix,
  matrix_access_type& matrix_access,
  matrix_run_config_type&  matrix_run_config,
  ::utilz::memory::buffer& b)
{
  using pointer    = typename utzmx::traits::matrix_traits<matrix_type>::pointer;
  using size_type  = typename utzmx::traits::matrix_traits<matrix_type>::size_type;
  using value_type = typename utzmx::traits::matrix_traits<matrix_type>::value_type;

  auto allocation_count = matrix_access.dimensions().max();
  auto allocation_size  = allocation_count * sizeof(value_type);

  matrix_run_config.mm_array_cur_row = reinterpret_cast<pointer>(b.allocate(allocation_size));
  matrix_run_config.mm_array_prv_col = reinterpret_cast<pointer>(b.allocate(allocation_size));
  matrix_run_config.mm_array_cur_col = reinterpret_cast<pointer>(b.allocate(allocation_size));
  matrix_run_config.mm_array_nxt_col = reinterpret_cast<pointer>(b.allocate(allocation_size));

  for (auto i = size_type(0); i < allocation_count; ++i) {
    matrix_run_config.mm_array_cur_row[i] = ::utilz::constants::infinity<value_type>();
    matrix_run_config.mm_array_prv_col[i] = ::utilz::constants::infinity<value_type>();
    matrix_run_config.mm_array_cur_col[i] = ::utilz::constants::infinity<value_type>();
    matrix_run_config.mm_array_nxt_col[i] = ::utilz::constants::infinity<value_type>();
  }
};

__hack_noinline
void
down(
  matrix_type&        matrix,
  matrix_access_type& matrix_access,
  matrix_run_config_type&  matrix_run_config,
  ::utilz::memory::buffer& b)
{
  using value_type = typename utzmx::traits::matrix_traits<matrix_type>::value_type;
  using alptr_type = typename ::utilz::memory::buffer::pointer;

  auto allocation_count = matrix_access.dimensions().max();
  auto allocation_size  = allocation_count * sizeof(value_type);

  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.mm_array_cur_row), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.mm_array_prv_col), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.mm_array_cur_col), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(matrix_run_config.mm_array_nxt_col), allocation_size);
}

__hack_noinline
void
run(
  matrix_type& m,
  matrix_run_config_type& run_config)
{
  using size_type  = typename utzmx::traits::matrix_traits<matrix_type>::size_type;
  using value_type = typename utzmx::traits::matrix_traits<matrix_type>::value_type;

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

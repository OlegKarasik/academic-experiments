#pragma once

#define APSP_ALG_MATRIX_CLUSTERS

#define APSP_ALG_EXTRA_CONFIGURATION

#define APSP_ALG_EXTRA_CLUSTERS_CONFIGURATION
#define APSP_ALG_EXTRA_CLUSTERS_REARRANGEMENTS

#include <constants.hpp>
#include <matrix-manip.hpp>
#include <matrix-traits.hpp>

#include "portables/hacks/defines.h"

#include "measure.hpp"

#include "memory.hpp"
#include "matrix.hpp"

#include <thread>

template<typename T, typename A, typename U>
struct run_configuration
{
  using pointer = typename ::utilz::matrices::traits::matrix_traits<::utilz::matrices::rect_matrix<T, A>>::pointer;

  pointer mm_array_cur_row;
  pointer mm_array_prv_col;
  pointer mm_array_cur_col;
  pointer mm_array_nxt_row;

  size_t allocation_line;
  size_t allocation_size;
};

template<typename T, typename A, typename U>
void
calculate_diagonal(
  ::utilz::matrices::rect_matrix<T, A>& mm,
  run_configuration<T, A, U>& run_config)
{
  SCOPE_MEASURE_MILLISECONDS("DIAG");

  using size_type  = typename ::utilz::matrices::traits::matrix_traits<::utilz::matrices::rect_matrix<T, A>>::size_type;
  using value_type = typename ::utilz::matrices::traits::matrix_traits<::utilz::matrices::rect_matrix<T, A>>::value_type;

  run_config.mm_array_prv_col[0] = ::utilz::constants::infinity<value_type>();
  run_config.mm_array_nxt_row[0] = mm.at(0, 1);

  for (auto k = size_type(1); k < mm.height(); ++k) {
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

    if (k < (mm.height() - 1))
      run_config.mm_array_nxt_row[k] = mm.at(k, k + 1);
  }

  const auto x = mm.height() - size_type(1);
  for (auto i = size_type(0); i < x; ++i) {
    const auto ix = run_config.mm_array_prv_col[i];

    __hack_ivdep
    for (auto j = size_type(0); j < x; ++j)
      mm.at(i, j) = (std::min)(mm.at(i, j), ix + mm.at(x, j));
  }
}

template<typename T, typename A>
void
calculate_vertical_fast(
  ::utilz::matrices::rect_matrix<T, A>& im,
  ::utilz::matrices::rect_matrix<T, A>& mm,
  auto bridges)
{
  SCOPE_MEASURE_MILLISECONDS("VERT");

  using size_type = typename ::utilz::matrices::traits::matrix_traits<::utilz::matrices::rect_matrix<T>>::size_type;

  const auto w = im.width();
  const auto h = im.height();
  const auto x = bridges[0];

  for (auto k = x + size_type(1); k < w; ++k) {
    const auto z = k - size_type(1);
    for (auto i = size_type(0); i < h; ++i) {
      const auto iz = im.at(i, z);

      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        im.at(i, j) = (std::min)(im.at(i, j), iz + mm.at(z, j));

      __hack_ivdep
      for (auto j = x; j < k; ++j) {
        im.at(i, j) = (std::min)(im.at(i, j), iz + mm.at(z, j));
        im.at(i, k) = (std::min)(im.at(i, k), im.at(i, j) + mm.at(j, k));
      }
    }
  }

  const auto z = w - size_type(1);
  for (auto i = size_type(0); i < h; ++i) {
    __hack_ivdep
    for (auto j = size_type(0); j < z; ++j)
      im.at(i, j) = (std::min)(im.at(i, j), im.at(i, z) + mm.at(z, j));
  }
};

template<typename T, typename A>
void
calculate_vertical(
  ::utilz::matrices::rect_matrix<T, A>& ij,
  ::utilz::matrices::rect_matrix<T, A>& ik,
  ::utilz::matrices::rect_matrix<T, A>& kj,
  auto bridges)
{
  SCOPE_MEASURE_MILLISECONDS("VERT");

  using size_type = typename ::utilz::matrices::traits::matrix_traits<::utilz::matrices::rect_matrix<T>>::size_type;

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
calculate_horizontal_fast(
  ::utilz::matrices::rect_matrix<T, A>& mi,
  ::utilz::matrices::rect_matrix<T, A>& mm,
  auto bridges)
{
  SCOPE_MEASURE_MILLISECONDS("HORZ");

  using size_type = typename ::utilz::matrices::traits::matrix_traits<::utilz::matrices::rect_matrix<T>>::size_type;

  const auto w = mi.width();
  const auto h = mm.height();
  const auto x = bridges[0];

  for (auto k = x + size_type(1); k < h; ++k) {
    auto const z = k - size_type(1);
    for (auto i = size_type(0); i < x; ++i) {
      const auto iz = mm.at(i, z);

      __hack_ivdep
      for (auto j = size_type(0); j < w; ++j)
        mi.at(i, j) = (std::min)(mi.at(i, j), iz + mi.at(z, j));
    }
    for (auto i = x; i < k; ++i) {
      const auto iz = mm.at(i, z);
      const auto ki = mm.at(k, i);

      __hack_ivdep
      for (auto j = size_type(0); j < w; ++j) {
        mi.at(i, j) = (std::min)(mi.at(i, j), iz + mi.at(z, j));
        mi.at(k, j) = (std::min)(mi.at(k, j), ki + mi.at(i, j));
      }
    }
  }

  const auto z = h - size_type(1);
  for (auto i = size_type(0); i < z; ++i) {
    const auto iz = mm.at(i, z);

    __hack_ivdep
    for (auto j = size_type(0); j < w; ++j)
      mi.at(i, j) = (std::min)(mi.at(i, j), iz + mi.at(z, j));
  }
};

template<typename T, typename A>
void
calculate_horizontal(
  ::utilz::matrices::rect_matrix<T, A>& ij,
  ::utilz::matrices::rect_matrix<T, A>& ik,
  ::utilz::matrices::rect_matrix<T, A>& kj,
  auto bridges)
{
  SCOPE_MEASURE_MILLISECONDS("HORZ");

  using size_type = typename ::utilz::matrices::traits::matrix_traits<::utilz::matrices::rect_matrix<T>>::size_type;

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
calculate_peripheral(
  ::utilz::matrices::rect_matrix<T, A>& ij,
  ::utilz::matrices::rect_matrix<T, A>& ik,
  ::utilz::matrices::rect_matrix<T, A>& kj,
  auto bridges)
{
  SCOPE_MEASURE_MILLISECONDS("PERH");

  using size_type = typename ::utilz::matrices::traits::matrix_traits<::utilz::matrices::rect_matrix<T>>::size_type;

  const auto ij_w = ij.width();
  const auto ij_h = ij.height();

  for (auto i = size_type(0); i < ij_h; ++i)
    for (auto k : bridges)
      __hack_ivdep
      for (auto j = size_type(0); j < ij_w; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

__hack_noinline
void
up_clusters(
  ::utilz::matrices::clusters& clusters)
{
  for (auto& group : clusters.list()) {
    const auto input_count = std::ranges::count_if(
      group.list(),
      [](const auto& vertex) -> bool {
        return std::get<::utilz::matrices::clusters_vertex_flag>(vertex) & ::utilz::matrices::clusters_vertex_flag_input;
      });
    const auto output_count = std::ranges::count_if(
      group.list(),
      [](const auto& vertex) -> bool {
        return std::get<::utilz::matrices::clusters_vertex_flag>(vertex) & ::utilz::matrices::clusters_vertex_flag_output;
      });
    if (input_count > output_count) {
      group.sort(
        {
          ::utilz::matrices::clusters_vertex_flag_none,
          ::utilz::matrices::clusters_vertex_flag_output,
          ::utilz::matrices::clusters_vertex_flag_input
        });
    } else {
      group.sort(
        {
          ::utilz::matrices::clusters_vertex_flag_none,
          ::utilz::matrices::clusters_vertex_flag_input,
          ::utilz::matrices::clusters_vertex_flag_output
        });
    }
  }
}

template<typename T, typename A, typename U>
__hack_noinline
void
up(
  ::utilz::matrices::square_matrix<::utilz::matrices::rect_matrix<T, A>, U>& blocks,
  ::utilz::memory::buffer& b,
  run_configuration<T, A, U>& run_config)
{
  using size_type  = typename ::utilz::matrices::traits::matrix_traits<::utilz::matrices::square_matrix<::utilz::matrices::rect_matrix<T, A>, U>>::size_type;
  using value_type = typename ::utilz::matrices::traits::matrix_traits<::utilz::matrices::square_matrix<::utilz::matrices::rect_matrix<T, A>, U>>::value_type;

  ::utilz::matrices::procedures::matrix_get_dimensions<::utilz::matrices::square_matrix<::utilz::matrices::rect_matrix<T, A>, U>> sz;

#ifdef _OPENMP
  auto allocation_mulx = std::thread::hardware_concurrency();
#else
  auto allocation_mulx = 1;
#endif

  auto allocation_line = sz(blocks).h();
  auto allocation_size = allocation_line * sizeof(value_type) * allocation_mulx;

  run_config.allocation_line = allocation_line;
  run_config.allocation_size = allocation_size;
  run_config.mm_array_cur_row = reinterpret_cast<T*>(b.allocate(allocation_size));
  run_config.mm_array_prv_col = reinterpret_cast<T*>(b.allocate(allocation_size));
  run_config.mm_array_cur_col = reinterpret_cast<T*>(b.allocate(allocation_size));
  run_config.mm_array_nxt_row = reinterpret_cast<T*>(b.allocate(allocation_size));

  for (size_type i = size_type(0); i < allocation_line * allocation_mulx; ++i) {
    run_config.mm_array_cur_row[i] = ::utilz::constants::infinity<value_type>();
    run_config.mm_array_prv_col[i] = ::utilz::constants::infinity<value_type>();
    run_config.mm_array_cur_col[i] = ::utilz::constants::infinity<value_type>();
    run_config.mm_array_nxt_row[i] = ::utilz::constants::infinity<value_type>();
  }
};

template<typename T, typename A, typename U>
__hack_noinline
void
down(
  ::utilz::matrices::square_matrix<::utilz::matrices::rect_matrix<T, A>, U>& blocks,
  ::utilz::memory::buffer& b,
  run_configuration<T, A, U>& run_config)
{
  using alptr_type = typename ::utilz::memory::buffer::pointer;

  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_cur_row), run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_prv_col), run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_cur_col), run_config.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(run_config.mm_array_nxt_row), run_config.allocation_size);
}

template<typename T, typename A, typename U>
__hack_noinline
void
run(
  ::utilz::matrices::square_matrix<utilz::matrices::rect_matrix<T, A>, U>& blocks,
  run_configuration<T, A, U>& run_config,
  ::utilz::matrices::clusters& clusters)
{
  using size_type = typename ::utilz::matrices::traits::matrix_traits<utilz::matrices::square_matrix<utilz::matrices::square_matrix<T, A>, U>>::size_type;

#ifdef _OPENMP
  #pragma omp parallel default(none) shared(blocks, run_config, clusters)
#endif
  {
#ifdef _OPENMP
  #pragma omp single
#endif
    {
      for (auto m = size_type(0); m < blocks.size(); ++m) {
        auto& mm = blocks.at(m, m);

        calculate_diagonal(mm, run_config);

        auto input_positions  = clusters.get_input_bridges_positions(m);
        auto output_positions = clusters.get_output_bridges_positions(m);

        for (auto i = size_type(0); i < blocks.size(); ++i) {
          if (i != m) {
            auto& im = blocks.at(i, m);
            auto& mi = blocks.at(m, i);

            if (input_positions.size() > output_positions.size()) {
              if (!input_positions.empty()) {
#ifdef _OPENMP
  #pragma omp task untied default(none) shared(im, mm, input_positions)
#endif
                calculate_vertical_fast(im, mm, input_positions);
              }

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(mi, mm, output_positions)
#endif
              calculate_horizontal(mi, mm, mi, output_positions);
            } else {
#ifdef _OPENMP
  #pragma omp task untied default(none) shared(im, mm, input_positions)
#endif
              calculate_vertical(im, im, mm, input_positions);

              if (!output_positions.empty()) {
#ifdef _OPENMP
  #pragma omp task untied default(none) shared(mi, mm, output_positions)
#endif
                calculate_horizontal_fast(mi, mm, output_positions);
              }
            }
          }
        }
#ifdef _OPENMP
  #pragma omp taskwait
#endif
        auto min_positions = input_positions.size() < output_positions.size()
          ? input_positions
          : output_positions;

        if (min_positions.size() == 0)
          continue;

        for (auto i = size_type(0); i < blocks.size(); ++i) {
          if (i != m) {
            auto& im = blocks.at(i, m);
            for (auto j = size_type(0); j < blocks.size(); ++j) {
              if (j != m) {
                auto& ij = blocks.at(i, j);
                auto& mj = blocks.at(m, j);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(ij, im, mj, min_positions)
#endif
                calculate_peripheral(ij, im, mj, min_positions);
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

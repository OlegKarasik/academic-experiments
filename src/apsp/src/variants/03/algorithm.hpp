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
  using pointer = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T>>::pointer;

  pointer mck;
  pointer drk;
  pointer mrk;
  pointer wrk;
  pointer ckb1;
  pointer ck1b1;
  pointer ckb1w;
  pointer ckb3w;

  size_t allocation_line;
  size_t allocation_size;
};

template<typename T, typename A, typename U>
__hack_noinline support_arrays<T>
up(::utilz::square_shape<utilz::square_shape<T, A>, U>& blocks, ::utilz::memory::buffer& b)
{
  using pointer    = typename ::utilz::traits::square_shape_traits<utilz::square_shape<utilz::square_shape<T, A>, U>>::pointer;
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<utilz::square_shape<T, A>, U>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<utilz::square_shape<T, A>, U>>::value_type;

  ::utilz::procedures::square_shape_get_size<utilz::square_shape<utilz::square_shape<T, A>, U>> sz;
  ::utilz::procedures::square_shape_at<utilz::square_shape<utilz::square_shape<T, A>, U>> at;

#ifdef _OPENMP
  auto allocation_mulx = std::thread::hardware_concurrency();
#else
  auto allocation_mulx = 1;
#endif

  auto allocation_line = sz(blocks);
  auto allocation_size = allocation_line * sizeof(value_type) * allocation_mulx;

  support_arrays<T> arrays;

  arrays.allocation_line = allocation_line;
  arrays.allocation_size = allocation_size;
  arrays.mck = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.drk = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.mrk = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.wrk = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.ckb1 = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.ck1b1 = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.ckb1w = reinterpret_cast<T*>(b.allocate(allocation_size));
  arrays.ckb3w = reinterpret_cast<T*>(b.allocate(allocation_size));

  // The algorithm requires that all self-loops have non "infinite" value. This
  // doesn't affect correctness of calculations.
  //
  for (size_type i = size_type(0); i < sz(blocks); ++i) {
    if (at(blocks, i, i) == ::apsp::constants::infinity<value_type>())
      at(blocks, i, i) = size_type(0);
  }

  for (size_type i = size_type(0); i < allocation_line * allocation_mulx; ++i) {
    arrays.mck[i] = ::apsp::constants::infinity<value_type>();
    arrays.drk[i] = ::apsp::constants::infinity<value_type>();
    arrays.mrk[i] = ::apsp::constants::infinity<value_type>();
    arrays.wrk[i] = ::apsp::constants::infinity<value_type>();
    arrays.ckb1[i] = ::apsp::constants::infinity<value_type>();
    arrays.ck1b1[i] = ::apsp::constants::infinity<value_type>();
    arrays.ckb1w[i] = ::apsp::constants::infinity<value_type>();
    arrays.ckb3w[i] = ::apsp::constants::infinity<value_type>();
  }

  return arrays;
};

template<typename T, typename A, typename U>
__hack_noinline void
down(::utilz::square_shape<utilz::square_shape<T, A>, U>& blocks, ::utilz::memory::buffer& b, support_arrays<T> o)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<utilz::square_shape<T, A>, U>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<utilz::square_shape<T, A>, U>>::value_type;

  using alptr_type = typename ::utilz::memory::buffer::pointer;

  ::utilz::procedures::square_shape_get_size<utilz::square_shape<utilz::square_shape<T, A>, U>> sz;
  ::utilz::procedures::square_shape_at<utilz::square_shape<utilz::square_shape<T, A>, U>> at;

  b.deallocate(reinterpret_cast<alptr_type>(o.mck), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.drk), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mrk), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.wrk), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.ckb1), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.ck1b1), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.ckb1w), o.allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.ckb3w), o.allocation_size);

  // Restoring the matrix to a state where self-loop is represented as
  // infinity instead of 0.
  //
  for (size_type i = size_type(0); i < sz(blocks); ++i)
    if (at(blocks, i, i) == size_type(0))
      at(blocks, i, i) = ::apsp::constants::infinity<value_type>();
}

template<typename T, typename A>
void
calculate_diagonal(::utilz::square_shape<T, A>& mm, support_arrays<T>& support_arrays)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;

  support_arrays.drk[0] = ::apsp::constants::infinity<value_type>();
  support_arrays.wrk[0] = mm.at(0, 1);

  for (auto k = size_type(1); k < mm.size(); ++k) {
    for (auto i = size_type(0); i < k; ++i)
      support_arrays.mck[i] = ::apsp::constants::infinity<value_type>();

    for (auto i = size_type(0); i < k; ++i) {
      const auto x = mm.at(k, i);
      const auto z = support_arrays.drk[i];

      auto minimum = ::apsp::constants::infinity<value_type>();

      __hack_ivdep
      for (auto j = size_type(0); j < k; ++j) {
        mm.at(i, j) = (std::min)(mm.at(i, j), z + mm.at(k - 1, j));

        minimum = (std::min)(minimum, mm.at(i, j) + support_arrays.wrk[j]);
        support_arrays.mck[j] = (std::min)(support_arrays.mck[j], mm.at(i, j) + x);
      }
      support_arrays.mrk[i] = minimum;
    }

    for (auto i = size_type(0); i < k; ++i) {
      mm.at(k, i) = support_arrays.mck[i];
      mm.at(i, k) = support_arrays.mrk[i];

      support_arrays.drk[i] = support_arrays.mrk[i];
      support_arrays.wrk[i] = mm.at(i, k + 1);
    }

    if (k < (mm.size() - 1))
      support_arrays.wrk[k] = mm.at(k, k + 1);
  }

  const auto x = mm.size() - size_type(1);
  for (auto i = size_type(0); i < x; ++i) {
    const auto ix = support_arrays.drk[i];

    __hack_ivdep
    for (auto j = size_type(0); j < x; ++j)
      mm.at(i, j) = (std::min)(mm.at(i, j), ix + mm.at(x, j));
  }
}

template<typename T, typename A>
void
calculate_horizontal(::utilz::square_shape<T, A>& im, ::utilz::square_shape<T, A>& mm, support_arrays<T>& support_arrays)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;
  using pointer    = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::pointer;

#ifdef _OPENMP
  auto allocation_shift = support_arrays.allocation_line * omp_get_thread_num();
#else
  auto allocation_shift = 0;
#endif

  pointer im_array_prv_column = support_arrays.drk + allocation_shift;
  pointer im_array_cur_column = support_arrays.mck + allocation_shift;
  pointer support_arrays_ckb1w = support_arrays.ckb1w + allocation_shift;
  pointer support_arrays_ckb3w = support_arrays.ckb3w + allocation_shift;

  size_type  i, j, k, k1;
  value_type  minr, sum0, sum1;
  pointer pCkb1w  = support_arrays.ckb1w + allocation_shift,
          pCkb3w  = support_arrays.ckb3w + allocation_shift,
          pC, priB1_j, prk1B3, prk1B3_j, pckB3w_j, pCkB1_i, ckB1w_i, pckB3w_i;

  value_type  pCk1B1_i;
  for (i = 0; i < im.size(); ++i) {
    im_array_prv_column[i] = ::apsp::constants::infinity<value_type>();
    pCkb1w[i] = im.at(i, 1);
    pCkb3w[i] = mm.at(i, 1);
  }
  for (k = 1; k < im.size(); ++k) {
    k1     = k - 1;
    prk1B3 = mm.at(k1);
    for (i = 0; i < im.size(); ++i) {
      minr     = im.at(i, k);
      priB1_j  = im.at(i);
      prk1B3_j = prk1B3;
      pCk1B1_i = im_array_prv_column[i];
      pckB3w_j = pCkb3w;
      for (j = 0; j < k; ++j, ++prk1B3_j, ++priB1_j, ++pckB3w_j) {
        sum1 = pCk1B1_i + *prk1B3_j;
        if (*priB1_j > sum1)
          *priB1_j = sum1;
        sum0 = *priB1_j + *pckB3w_j;
        if (minr > sum0)
          minr = sum0;
      }
      im_array_cur_column[i] = minr;
    }
    priB1_j  = im.at(0) + k;
    pckB3w_j = mm.at(0) + (k + 1);
    pCkB1_i  = im_array_cur_column;
    ckB1w_i  = pCkb1w;
    pckB3w_i = pCkb3w;
    for (i = 0; i < im.size(); ++i, priB1_j += im.size(), pckB3w_j += im.size(), ++pCkB1_i, ++ckB1w_i, ++pckB3w_i) {
      *priB1_j  = *pCkB1_i;
      *ckB1w_i  = priB1_j[1];
      *pckB3w_i = *pckB3w_j;
    }
    std::swap(im_array_prv_column, im_array_cur_column);
  }

  const auto x = im.size();
  const auto z = k - 1;
  for (auto i = size_type(0); i < x; ++i) {
    auto value = im_array_prv_column[i];

    __hack_ivdep
    for (auto j = size_type(0); j < z; ++j) {
      im.at(i, j) = (std::min)(im.at(i, j), value + mm.at(z, j));
    }
  }
}

template<typename T, typename A>
void
calculate_vertical(::utilz::square_shape<T, A>& mi, ::utilz::square_shape<T, A>& mm, support_arrays<T>& support_arrays)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;
  using pointer    = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::pointer;

#ifdef _OPENMP
  auto allocation_shift = support_arrays.allocation_line * omp_get_thread_num();
#else
  auto allocation_shift = 0;
#endif

  size_type  k, k1;
  value_type  sum0, sum1;
  value_type  ck1B2i, rkB2i;
  pointer prkB1, prkB1_j, priB1, prk1B1, prk1B1_j, prkB2, pck1B2i, priB2;

  pointer pCkb1 = support_arrays.ckb1 + allocation_shift;

  pCkb1[0] = mm.at(0, 0);
  for (k = size_type(1); k < mi.size(); ++k) {
    k1     = k - 1;
    prkB1  = mi.at(k);
    prk1B1 = mi.at(k1);
    prkB2  = mm.at(k);
    for (auto i = size_type(0); i < k; ++i) {
      ck1B2i   = pCkb1[i];
      prkB1_j  = prkB1;
      priB1    = mi.at(i);
      prk1B1_j = prk1B1;
      rkB2i    = prkB2[i];
      for (auto j = size_type(0); j < mi.size(); ++j, ++prkB1_j, ++priB1, ++prk1B1_j) {
        sum1 = ck1B2i + *prk1B1_j;
        if (*priB1 > sum1)
          *priB1 = sum1;
        sum0 = rkB2i + *priB1;
        if (*prkB1_j > sum0)
          *prkB1_j = sum0;
      }
    }
    pck1B2i = pCkb1;
    priB2   = mm.at(0) + k;
    for (auto i = size_type(0); i < k; ++i, ++pck1B2i, priB2 += mi.size()) {
      *pck1B2i = *priB2;
    }
  }

  const auto x = k - size_type(1);
  const auto z = mi.size();
  for (auto i = size_type(0); i < x; ++i) {
    const auto ix = pCkb1[i];

    __hack_ivdep
    for (auto j = size_type(0); j < z; ++j)
      mi.at(i, j) = (std::min)(mi.at(i, j), ix + mi.at(x, j));
  }
}

template<typename T, typename A>
void
calculate_peripheral(::utilz::square_shape<T, A>& ij, ::utilz::square_shape<T, A>& ik, ::utilz::square_shape<T, A>& kj)
{
  using size_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T>>::size_type;

  const auto x = ij.size();
  for (auto k = size_type(0); k < x; ++k)
    for (auto i = size_type(0); i < x; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
}

template<typename T, typename A, typename U>
__hack_noinline void
run(::utilz::square_shape<utilz::square_shape<T, A>, U>& blocks, support_arrays<T>& support_arrays)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
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

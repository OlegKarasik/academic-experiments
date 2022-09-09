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
calculate_diagonal_block(::utilz::square_shape<T, A>& block, support_arrays<T>& support_arrays)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;
  using pointer    = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::pointer;

  pointer    pDck;
  pointer    pDij;
  value_type minR;
  value_type sumR, sumC, sumDij;
  value_type drki;
  pointer    pBik;
  pointer    pBki;
  value_type bki;
  pointer    pdck, pmrk, pwrk, pmck;

  support_arrays.drk[0] = ::apsp::constants::infinity<value_type>();
  support_arrays.wrk[0] = block.at(0, 1);
  for (size_type k = size_type(1); k < block.size(); ++k) {
    pDck = block.at(k - 1);
    for (size_type i = size_type(0); i < k; ++i)
      support_arrays.mck[i] = ::apsp::constants::infinity<value_type>();
    pBki = block.at(k);
    pmrk = &support_arrays.mrk[0];
    for (size_type i = 0; i < k; ++i, ++pmrk) {
      minR = ::apsp::constants::infinity<value_type>();
      bki  = pBki[i];
      drki = support_arrays.drk[i];
      pDij = block.at(i);
      pdck = pDck;
      for (size_type j = 0; j < k; ++j, ++pDij, ++pdck) {
        sumDij = drki + *pdck;
        if (*pDij > sumDij)
          *pDij = sumDij;
        sumR = *pDij + support_arrays.wrk[j];
        if (minR > sumR)
          minR = sumR;
        sumC = *pDij + bki;
        if (support_arrays.mck[j] > sumC)
          support_arrays.mck[j] = sumC;
      }
      *pmrk = minR;
    }

    pBki = block.at(k);
    pBik = block.at(0) + k;
    pwrk = support_arrays.wrk;
    pmck = support_arrays.mck;
    for (size_type i = 0; i < k; ++i, ++pBki, pBik += block.size(), ++pwrk, ++pmck) {
      *pBki = *pmck;
      *pBik = support_arrays.drk[i] = support_arrays.mrk[i];
      *pwrk                         = pBik[1];
    }
    if (k < block.size() - 1)
      *pwrk = pBik[1];
  }

  for (size_type i = size_type(0); i < block.size() - size_type(1); ++i) {
    drki = support_arrays.drk[i];
    pDij = block.at(i);
    pdck = block.at(block.size() - 1);
    for (size_type j = 0; j < block.size() - size_type(1); ++j, ++pDij, ++pdck) {
      sumDij = drki + *pdck;
      if (*pDij > sumDij)
        *pDij = sumDij;
    }
  }
}

template<typename T, typename A>
void
BCA_C1(::utilz::square_shape<T, A>& b1, ::utilz::square_shape<T, A>& b3, support_arrays<T>& support_arrays)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;
  using pointer    = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::pointer;

#ifdef _OPENMP
  auto allocation_shift = support_arrays.allocation_line * omp_get_thread_num();
#else
  auto allocation_shift = 0;
#endif

  size_type  i, j, k, k1;
  value_type  minr, sum0, sum1;
  pointer pCk1B1_ = support_arrays.drk + allocation_shift,
          pCkB1_  = support_arrays.mck + allocation_shift,
          pCkb1w  = support_arrays.ckb1w + allocation_shift,
          pCkb3w  = support_arrays.ckb3w + allocation_shift,
          pC, priB1_j, prk1B3, prk1B3_j, pckB3w_j, pCkB1_i, ckB1w_i, pckB3w_i;

  value_type  pCk1B1_i;
  for (i = 0; i < b1.size(); ++i) {
    pCk1B1_[i] = ::apsp::constants::infinity<value_type>();
    pCkb1w[i] = b1.at(i, 1);
    pCkb3w[i] = b3.at(i, 1);
  }
  for (k = 1; k < b1.size(); ++k) {
    k1     = k - 1;
    prk1B3 = b3.at(k1);
    for (i = 0; i < b1.size(); ++i) {
      minr     = b1.at(i, k);
      priB1_j  = b1.at(i);
      prk1B3_j = prk1B3;
      pCk1B1_i = pCk1B1_[i];
      pckB3w_j = pCkb3w;
      for (j = 0; j < k; ++j, ++prk1B3_j, ++priB1_j, ++pckB3w_j) {
        sum1 = pCk1B1_i + *prk1B3_j;
        if (*priB1_j > sum1)
          *priB1_j = sum1;
        sum0 = *priB1_j + *pckB3w_j;
        if (minr > sum0)
          minr = sum0;
      }
      pCkB1_[i] = minr;
    }
    priB1_j  = b1.at(0) + k;
    pckB3w_j = b3.at(0) + (k + 1);
    pCkB1_i  = pCkB1_;
    ckB1w_i  = pCkb1w;
    pckB3w_i = pCkb3w;
    for (i = 0; i < b1.size(); ++i, priB1_j += b1.size(), pckB3w_j += b1.size(), ++pCkB1_i, ++ckB1w_i, ++pckB3w_i) {
      *priB1_j  = *pCkB1_i;
      *ckB1w_i  = priB1_j[1];
      *pckB3w_i = *pckB3w_j;
    }
    pC      = pCk1B1_;
    pCk1B1_ = pCkB1_;
    pCkB1_  = pC;
  }
  k1     = k - 1;
  prk1B3 = b3.at(k1);
  for (i = 0; i < b1.size(); ++i) {
    priB1_j  = b1.at(i);
    prk1B3_j = prk1B3;
    pCk1B1_i = pCk1B1_[i];
    for (j = 0; j < k1; ++j, ++priB1_j, ++prk1B3_j) {
      sum1 = pCk1B1_i + *prk1B3_j;
      if (*priB1_j > sum1)
        *priB1_j = sum1;
    }
  }
}

template<typename T, typename A>
void
BCA_C2(::utilz::square_shape<T, A>& b1, ::utilz::square_shape<T, A>& b2, support_arrays<T>& support_arrays)
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

  pCkb1[0] = b2.at(0, 0);
  for (k = size_type(1); k < b1.size(); ++k) {
    k1     = k - 1;
    prkB1  = b1.at(k);
    prk1B1 = b1.at(k1);
    prkB2  = b2.at(k);
    for (auto i = size_type(0); i < k; ++i) {
      ck1B2i   = pCkb1[i];
      prkB1_j  = prkB1;
      priB1    = b1.at(i);
      prk1B1_j = prk1B1;
      rkB2i    = prkB2[i];
      for (auto j = size_type(0); j < b1.size(); ++j, ++prkB1_j, ++priB1, ++prk1B1_j) {
        sum1 = ck1B2i + *prk1B1_j;
        if (*priB1 > sum1)
          *priB1 = sum1;
        sum0 = rkB2i + *priB1;
        if (*prkB1_j > sum0)
          *prkB1_j = sum0;
      }
    }
    pck1B2i = pCkb1;
    priB2   = b2.at(0) + k;
    for (auto i = size_type(0); i < k; ++i, ++pck1B2i, priB2 += b1.size()) {
      *pck1B2i = *priB2;
    }
  }
  k1     = k - 1;
  prk1B1 = b1.at(k1);
  for (auto i = size_type(0); i < k1; ++i) {
    ck1B2i   = pCkb1[i];
    priB1    = b1.at(i);
    prk1B1_j = prk1B1;
    for (auto j = 0; j < b1.size(); ++j, ++priB1, ++prk1B1_j) {
      sum1 = ck1B2i + *prk1B1_j;
      if (*priB1 > sum1)
        *priB1 = sum1;
    }
  }
}

template<typename T, typename A>
void
calculate_peripheral(
  ::utilz::square_shape<T, A>& b1,
  ::utilz::square_shape<T, A>& b2,
  ::utilz::square_shape<T, A>& b3,
  support_arrays<T>& support_arrays
)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;

  for (auto i = size_type(0); i < b1.size(); ++i) {
    for (auto k = size_type(0); k < b1.size(); ++k) {
      auto ik = b2.at(i, k);
      for (auto j = size_type(0); j < b1.size(); ++j) {
        auto sum = ik + b3.at(k, j);
        if (b1.at(i, j) > sum) {
          b1.at(i, j) = sum;
        }
      }
    }
  }
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
        auto& center = blocks.at(m, m);

        calculate_diagonal_block(center, support_arrays);
        for (auto i = size_type(0); i < blocks.size(); ++i) {
          if (i != m) {
            auto& x = blocks.at(i, m);
            auto& y = blocks.at(m, m);
            auto& z = blocks.at(m, i);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(x, y, support_arrays)
#endif
            BCA_C1(x, y, support_arrays);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(z, y, support_arrays)
#endif
            BCA_C2(z, y, support_arrays);
          }
        }
#ifdef _OPENMP
  #pragma omp taskwait
#endif
        for (auto i = size_type(0); i < blocks.size(); ++i) {
          if (i != m) {
            for (auto j = size_type(0); j < blocks.size(); ++j) {
              if (j != m) {
                auto& x = blocks.at(i, j);
                auto& y = blocks.at(i, m);
                auto& z = blocks.at(m, j);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(x, y, z, support_arrays)
#endif
                calculate_peripheral(x, y, z, support_arrays);
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

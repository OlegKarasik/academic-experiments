#pragma once

#define APSP_ALG_BLOCKED
#define APSP_ALG_WIND_UPDOWN

#include "memory.hpp"
#include "square-shape.hpp"

#include "constants.hpp"

template<typename T>
struct support_arrays
{
  using pointer = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T>>::pointer;

  pointer mck;
  pointer drk;
  pointer mrk;
  pointer wrk;
};

template<typename T, typename A>
__attribute__((noinline)) support_arrays<T>
wind_up_apsp(::utilz::square_shape<T, A>& m, ::utilz::memory::buffer& b)
{
  using pointer    = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::pointer;
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;

  auto allocation_size = m.size() * sizeof(value_type);

  support_arrays<T> arrays;

  arrays.mck = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.drk = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.mrk = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.wrk = reinterpret_cast<pointer>(b.allocate(allocation_size));

  // The algorithm requires that all self-loops have non "infinite" value. This
  // doesn't affect correctness of calculations.
  //
  for (size_type i = size_type(0); i < m.size(); ++i) {
    if (m.at(i, i) == ::apsp::constants::infinity<value_type>())
      m.at(i, i) = size_type(0);

    arrays.mck[i] = ::apsp::constants::infinity<value_type>();
    arrays.drk[i] = ::apsp::constants::infinity<value_type>();
    arrays.mrk[i] = ::apsp::constants::infinity<value_type>();
    arrays.wrk[i] = ::apsp::constants::infinity<value_type>();
  }

  return arrays;
};

template<typename T, typename A>
__attribute__((noinline)) void
wind_down_apsp(::utilz::square_shape<T, A>& m, ::utilz::memory::buffer& b, support_arrays<T> o)
{
  using size_type  = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;
  using value_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;

  using alptr_type = typename ::utilz::memory::buffer::pointer;

  auto allocation_size = m.size() * sizeof(value_type);

  b.deallocate(reinterpret_cast<alptr_type>(o.mck), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.drk), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.mrk), allocation_size);
  b.deallocate(reinterpret_cast<alptr_type>(o.wrk), allocation_size);

  // Restoring the matrix to a state where self-loop is represented as
  // infinity instead of 0.
  //
  for (size_type i = size_type(0); i < m.size(); ++i)
    if (m.at(i, i) == size_type(0))
      m.at(i, i) = ::apsp::constants::infinity<value_type>();
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
    pBik = &block.at(0)[k];
    pwrk = &support_arrays.wrk[0];
    pmck = &support_arrays.mck[0];

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

// void
// BCA_C1(int b1, int b3)
// { // ++++++++++++++++++++++++++++++++++++++++ To use !!!!!
//   int* B1 = BA[b1];
//   int* B3 = BA[b3];
//   int  i, j, k, k1;
//   int  minr, sum0, sum1;
//   int *pCk1B1_ = ck1B1_, *pCkB1_ = ckB1_, *pC, *priB1_j, *prk1B3, *prk1B3_j, *pckB3w_j, *pCkB1_i, *ckB1w_i, *pckB3w_i;
//   int  pCk1B1_i;
//   for (i = 0; i < B; ++i) {
//     pCk1B1_[i] = FMAX;
//     ckB1w_[i]  = B1[i * B + 1];
//     ckB3w_[i]  = B3[i * B + 1];
//   }
//   for (k = 1; k < B; ++k) {
//     k1     = k - 1;
//     prk1B3 = &B3[k1 * B];
//     for (i = 0; i < B; ++i) {
//       minr     = B1[i * B + k];
//       priB1_j  = &B1[i * B];
//       prk1B3_j = prk1B3;
//       pCk1B1_i = pCk1B1_[i];
//       pckB3w_j = ckB3w_;
//       for (j = 0; j < k; ++j, ++prk1B3_j, ++priB1_j, ++pckB3w_j) {
//         sum1 = pCk1B1_i + *prk1B3_j;
//         if (*priB1_j > sum1)
//           *priB1_j = sum1;
//         sum0 = *priB1_j + *pckB3w_j;
//         if (minr > sum0)
//           minr = sum0;
//       }
//       pCkB1_[i] = minr;
//     }
//     priB1_j  = B1 + k;
//     pckB3w_j = B3 + k + 1;
//     pCkB1_i  = pCkB1_;
//     ckB1w_i  = ckB1w_;
//     pckB3w_i = ckB3w_;
//     for (i = 0; i < B; ++i, priB1_j += B, pckB3w_j += B, ++pCkB1_i, ++ckB1w_i, ++pckB3w_i) {
//       *priB1_j  = *pCkB1_i;
//       *ckB1w_i  = priB1_j[1];
//       *pckB3w_i = *pckB3w_j;
//     }
//     pC      = pCk1B1_;
//     pCk1B1_ = pCkB1_;
//     pCkB1_  = pC;
//   }
//   k1     = k - 1;
//   prk1B3 = &B3[k1 * B];
//   for (i = 0; i < B; ++i) {
//     priB1_j  = &B1[i * B];
//     prk1B3_j = prk1B3;
//     pCk1B1_i = pCk1B1_[i];
//     for (j = 0; j < k1; ++j, ++priB1_j, ++prk1B3_j) {
//       sum1 = pCk1B1_i + *prk1B3_j;
//       if (*priB1_j > sum1)
//         *priB1_j = sum1;
//     }
//   }
// }
// void
// BCA_C2(int b1, int b2)
// { // ++++++++++++++++++++++++++++++++++++++++++++++++++++   For paper and use
//   int* B1 = BA[b1];
//   int* B2 = BA[b2];
//   int  j, i, k, k1;
//   int  sum0, sum1;
//   int  ck1B2i, rkB2i;
//   int *prkB1, *prkB1_j, *priB1, *prk1B1, *prk1B1_j, *prkB2, *pck1B2i, *priB2;
//   ck1B2[0] = B2[0];
//   for (k = 1; k < B; ++k) {
//     k1     = k - 1;
//     prkB1  = &B1[k * B];
//     prk1B1 = &B1[k1 * B];
//     prkB2  = &B2[k * B];
//     for (i = 0; i < k; ++i) {
//       ck1B2i   = ck1B2[i];
//       prkB1_j  = prkB1;
//       priB1    = &B1[i * B];
//       prk1B1_j = prk1B1;
//       rkB2i    = prkB2[i];
//       for (j = 0; j < B; ++j, ++prkB1_j, ++priB1, ++prk1B1_j) {
//         sum1 = ck1B2i + *prk1B1_j;
//         if (*priB1 > sum1)
//           *priB1 = sum1;
//         sum0 = rkB2i + *priB1;
//         if (*prkB1_j > sum0)
//           *prkB1_j = sum0;
//       }
//     }
//     pck1B2i = ck1B2;
//     priB2   = &B2[0] + k;
//     for (i = 0; i < k; ++i, ++pck1B2i, priB2 += B) {
//       *pck1B2i = *priB2;
//     }
//   }
//   k1     = k - 1;
//   prk1B1 = &B1[k1 * B];
//   for (i = 0; i < k1; ++i) {
//     ck1B2i   = ck1B2[i];
//     priB1    = &B1[i * B];
//     prk1B1_j = prk1B1;
//     for (j = 0; j < B; ++j, ++priB1, ++prk1B1_j) {
//       sum1 = ck1B2i + *prk1B1_j;
//       if (*priB1 > sum1)
//         *priB1 = sum1;
//     }
//   }
// }
// void
// BCA_P3(int b1, int b2, int b3)
// {
//   int* B1 = BA[b1];
//   int* B2 = BA[b2];
//   int* B3 = BA[b3];
//   int  i, j, k;
//   int  sum;
//   int *k_rowB3, *pkj_row;
//   int *i_rowB1, *i_rowB1j, *i_rowB2;
//   int  ik;
//   for (i = 0; i < B; ++i) {
//     i_rowB1 = &B1[i * B];
//     i_rowB2 = &B2[i * B];
//     for (k = 0; k < B; ++k) {
//       k_rowB3  = &B3[k * B];
//       ik       = i_rowB2[k];
//       i_rowB1j = i_rowB1;
//       for (j = 0; j < B; ++j, ++k_rowB3, ++i_rowB1j) {
//         sum = ik + *k_rowB3;
//         if (*i_rowB1j > sum) {
//           *i_rowB1j = sum;
//         }
//       }
//     }
//   }
// }

template<typename T, typename A, typename U>
__attribute__((noinline)) void
calculate_apsp(::utilz::square_shape<utilz::square_shape<T, A>, U>& blocks, support_arrays<T>& support_arrays)
{
  int i, j, m;
  for (m = 0; m < blocks.size(); ++m) {
    auto& center = blocks.at(m, m);

    calculate_diagonal_block(center);
    for (i = 0; i < blocks.size(); ++i) {
      if (i != m) {
        //BCA_C1(i * M + m, m * M + m);
        //BCA_C2(m * M + i, m * M + m);
      }
    }
    for (i = 0; i < blocks.size(); ++i) {
      if (i != m) {
        for (j = 0; j < blocks.size(); ++j) {
          if (j != m) {
            //BCA_P3(i * M + j, i * M + m, m * M + j);
          }
        }
      }
    }
  }
};

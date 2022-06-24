#pragma once

#define APSP_ALG_HAS_OPTIONS

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
up(::utilz::square_shape<T, A>& m, ::utilz::memory::buffer& b)
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
down(::utilz::square_shape<T, A>& m, ::utilz::memory::buffer& b, support_arrays<T> o)
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
__attribute__((noinline)) void
run(::utilz::square_shape<T, A>& m, support_arrays<T>& support_arrays)
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
  support_arrays.wrk[0] = m.at(0, 1);

  for (size_type k = size_type(1); k < m.size(); ++k) {
    pDck = m.at(k - 1);
    for (size_type i = size_type(0); i < k; ++i)
      support_arrays.mck[i] = ::apsp::constants::infinity<value_type>();

    pBki = m.at(k);
    pmrk = &support_arrays.mrk[0];
    for (size_type i = 0; i < k; ++i, ++pmrk) {
      minR = ::apsp::constants::infinity<value_type>();
      bki  = pBki[i];
      drki = support_arrays.drk[i];
      pDij = m.at(i);
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

    pBki = m.at(k);
    pBik = &m.at(0)[k];
    pwrk = &support_arrays.wrk[0];
    pmck = &support_arrays.mck[0];

    for (size_type i = 0; i < k; ++i, ++pBki, pBik += m.size(), ++pwrk, ++pmck) {
      *pBki = *pmck;
      *pBik = support_arrays.drk[i] = support_arrays.mrk[i];
      *pwrk                         = pBik[1];
    }

    if (k < m.size() - 1)
      *pwrk = pBik[1];
  }
  for (size_type i = size_type(0); i < m.size() - size_type(1); ++i) {
    drki = support_arrays.drk[i];
    pDij = m.at(i);
    pdck = m.at(m.size() - 1);

    for (size_type j = 0; j < m.size() - size_type(1); ++j, ++pDij, ++pdck) {
      sumDij = drki + *pdck;
      if (*pDij > sumDij)
        *pDij = sumDij;
    }
  }
};

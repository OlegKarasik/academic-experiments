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

  pointer mck;
  pointer drk;
  pointer mrk;
  pointer wrk;
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

  arrays.mck = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.drk = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.mrk = reinterpret_cast<pointer>(b.allocate(allocation_size));
  arrays.wrk = reinterpret_cast<pointer>(b.allocate(allocation_size));

  // The algorithm requires that all self-loops have non "infinite" value. This
  // doesn't affect correctness of calculations.
  //
  for (auto i = size_type(0); i < m.size(); ++i) {
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
__hack_noinline void
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

  support_arrays.drk[0] = ::apsp::constants::infinity<value_type>();
  support_arrays.wrk[0] = m.at(0, 1);

  for (auto k = size_type(1); k < m.size(); ++k) {
    for (auto i = size_type(0); i < k; ++i)
      support_arrays.mck[i] = ::apsp::constants::infinity<value_type>();

    for (auto i = size_type(0); i < k; ++i) {
      const auto x = m.at(k, i);
      const auto z = support_arrays.drk[i];

      auto minimum = ::apsp::constants::infinity<value_type>();

      __hack_ivdep
      for (auto j = size_type(0); j < k; ++j) {
        m.at(i, j) = (std::min)(m.at(i, j), z + m.at(k - 1, j));

        minimum = (std::min)(minimum, m.at(i, j) + support_arrays.wrk[j]);
        support_arrays.mck[j] = (std::min)(support_arrays.mck[j], m.at(i, j) + x);
      }
      support_arrays.mrk[i] = minimum;
    }

    for (auto i = size_type(0); i < k; ++i) {
      m.at(k, i) = support_arrays.mck[i];
      m.at(i, k) = support_arrays.mrk[i];

      support_arrays.drk[i] = support_arrays.mrk[i];
      support_arrays.wrk[i] = m.at(i, k + 1);
    }

    if (k < (m.size() - 1))
      support_arrays.wrk[k] = m.at(k, k + 1);
  }

  const auto x = m.size() - size_type(1);
  for (auto i = size_type(0); i < x; ++i) {
    const auto v = support_arrays.drk[i];

    __hack_ivdep
    for (auto j = size_type(0); j < x; ++j)
      m.at(i, j) = (std::min)(m.at(i, j), v + m.at(x, j));
  }
};

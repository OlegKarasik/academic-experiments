#pragma once

#define APSP_ALG_HAS_BLOCKS

#include "portables/hacks/defines.h"

#include "square-shape.hpp"

template<typename T, typename A>
void
calculate_block(::utilz::square_shape<T, A>& ij, ::utilz::square_shape<T, A>& ik, ::utilz::square_shape<T, A>& kj)
{
  using size_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<T>>::size_type;

  const auto x = ij.size();
  for (auto k = size_type(0); k < x; ++k)
    for (auto i = size_type(0); i < x; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

template<typename T, typename A, typename U>
__hack_noinline void
run(::utilz::square_shape<utilz::square_shape<T, A>, U>& blocks)
{
  using size_type = typename ::utilz::traits::square_shape_traits<utilz::square_shape<utilz::square_shape<T, A>, U>>::size_type;

#ifdef _OPENMP
  #pragma omp parallel default(none) shared(blocks)
#endif
  {
#ifdef _OPENMP
  #pragma omp single
#endif
    {
      for (auto b = size_type(0); b < blocks.size(); ++b) {
        auto& center = blocks.at(b, b);

        calculate_block(center, center, center);

        for (auto i = size_type(0); i < b; ++i) {
          auto& north = blocks.at(i, b);
          auto& west  = blocks.at(b, i);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(north, center)
#endif
          calculate_block(north, north, center);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(west, center)
#endif
          calculate_block(west, center, west);
        };
        for (auto i = (b + size_type(1)); i < blocks.size(); ++i) {
          auto& south = blocks.at(i, b);
          auto& east  = blocks.at(b, i);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(south, center)
#endif
          calculate_block(south, south, center);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(east, center)
#endif
          calculate_block(east, center, east);
        };
#ifdef _OPENMP
  #pragma omp taskwait
#endif

        for (auto i = size_type(0); i < b; ++i) {
          auto& north = blocks.at(i, b);
          for (auto j = size_type(0); j < b; ++j) {
            auto& ij = blocks.at(i, j);
            auto& bj = blocks.at(b, j);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(ij, north, bj)
#endif
            calculate_block(ij, north, bj);
          };

          for (auto j = (b + size_type(1)); j < blocks.size(); ++j) {
            auto& ij = blocks.at(i, j);
            auto& bj = blocks.at(b, j);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(ij, north, bj)
#endif
            calculate_block(ij, north, bj);
          };
        };
        for (auto i = (b + size_type(1)); i < blocks.size(); ++i) {
          auto& south = blocks.at(i, b);
          for (auto j = size_type(0); j < b; ++j) {
            auto& ij = blocks.at(i, j);
            auto& bj = blocks.at(b, j);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(ij, south, bj)
#endif
            calculate_block(ij, south, bj);
          };

          for (auto j = (b + size_type(1)); j < blocks.size(); ++j) {
            auto& ij = blocks.at(i, j);
            auto& bj = blocks.at(b, j);

#ifdef _OPENMP
  #pragma omp task untied default(none) shared(ij, south, bj)
#endif
            calculate_block(ij, south, bj);
          };
        };
#ifdef _OPENMP
  #pragma omp taskwait
#endif
      };
    }
  }
};

#pragma once

#include "square-shape.hpp"

template<typename T, typename A>
void
calculate_block(utilz::square_shape<T, A>& ij, utilz::square_shape<T, A>& ik, utilz::square_shape<T, A>& kj)
{
  using size_type = typename utilz::traits::square_shape_traits<utilz::square_shape<T>>::size_type;

  for (size_type k = size_type(0); k < ij.size(); ++k)
    for (size_type i = size_type(0); i < ij.size(); ++i)
      for (size_type j = size_type(0); j < ij.size(); ++j)
        ij.at(i, j) = std::min(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

template<typename T, typename A, typename U>
__attribute__((noinline)) void
calculate_apsp(utilz::square_shape<utilz::square_shape<T, A>, U>& blocks)
{
  using size_type = typename utilz::traits::square_shape_traits<utilz::square_shape<utilz::square_shape<T, A>, U>>::size_type;

  for (size_type b = size_type(0); b < blocks.size(); ++b) {
    auto& center = blocks.at(b, b);

    calculate_block(center, center, center);

    for (size_type i = size_type(0); i < b; ++i) {
      auto& north = blocks.at(i, b);
      auto& west  = blocks.at(b, i);

      calculate_block(north, north, center);
      calculate_block(west, center, west);
    };
    for (size_type i = (b + size_type(1)); i < blocks.size(); ++i) {
      auto& south = blocks.at(i, b);
      auto& east  = blocks.at(b, i);

      calculate_block(south, south, center);
      calculate_block(east, center, east);
    };

    for (size_type i = size_type(0); i < b; ++i) {
      auto& north = blocks.at(i, b);
      for (size_type j = size_type(0); j < b; ++j) {
        auto& ij = blocks.at(i, j);
        auto& bj = blocks.at(b, j);

        calculate_block(ij, north, bj);
      };

      for (size_type j = (b + size_type(1)); j < blocks.size(); ++j) {
        auto& ij = blocks.at(i, j);
        auto& bj = blocks.at(b, j);

        calculate_block(ij, north, bj);
      };
    };
    for (size_type i = (b + size_type(1)); i < blocks.size(); ++i) {
      auto& south = blocks.at(i, b);
      for (size_type j = size_type(0); j < b; ++j) {
        auto& ij = blocks.at(i, j);
        auto& bj = blocks.at(b, j);

        calculate_block(ij, south, bj);
      };

      for (size_type j = (b + size_type(1)); j < blocks.size(); ++j) {
        auto& ij = blocks.at(i, j);
        auto& bj = blocks.at(b, j);

        calculate_block(ij, south, bj);
      };
    };
  };
};

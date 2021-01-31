#pragma once

#include "../../utilz/square-shape.hpp"

template<typename T>
void
impl(utilz::square_shape<T>& matrix)
{
  for (auto k = 0; k < matrix.s(); ++k) {
    auto k_row = matrix(k);

    for (auto i = 0; i < matrix.s(); ++i) {
      auto i_row = matrix(i);

      auto ik = i_row[k];
      for (auto j = 0; j < matrix.s(); ++j) {
        auto distance = ik + k_row[j];
        if (i_row[j] > distance)
          i_row[j] = distance;
      };
    };
  };
};

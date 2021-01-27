#pragma once

#include "../../utilz/rect-shape.hpp"

void
base_impl(utilz::rect_shape<long>& matrix)
{
  for (size_t k = 0; k < matrix.h(); ++k) {
    long* k_row = matrix(k);

    for (size_t i = 0; i < matrix.h(); ++i) {
      long* i_row = matrix(i);

      long ik = i_row[k];
      for (size_t j = 0; j < matrix.w(); ++j) {
        long distance = ik + k_row[j];
        if (i_row[j] > distance)
          i_row[j] = distance;
      };
    };
  };
};

void
o1_impl(utilz::rect_shape<long>& matrix)
{
  for (size_t k = 0; k < matrix.h(); ++k) {
    long* k_row = matrix(k);

    for (size_t i = 0; i < matrix.h(); ++i) {
      long* i_row = matrix(i);

      long ik = i_row[k];
      for (size_t j = 0; j < matrix.w(); ++j) {
        long distance = ik + k_row[j];
        if (i_row[j] > distance)
          i_row[j] = distance;
      };
    };
  };
};

void
o2_impl(utilz::rect_shape<long>& matrix)
{
  long* begin = matrix.begin();
  long* end   = matrix.end();

  long* k_row  = begin;
  long* ik_row = begin;
  for (; k_row != end; ++ik_row) {
    long* k_row_l = k_row + matrix.w();

    long* i_row   = begin;
    long* ik_cell = ik_row;
    for (; i_row != end; ik_cell += matrix.w()) {
      for (long* k_row_f = k_row; k_row_f != k_row_l; ++i_row, ++k_row_f) {
        long distance = *ik_cell + *k_row_f;
        if (*i_row > distance)
          *i_row = distance;
      };
    }

    k_row = k_row_l;
  }
};

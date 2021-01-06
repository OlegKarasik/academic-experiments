#include <algorithm>
#include <iostream>

#include "../../utilz/square_shape.h"

using namespace std;
using namespace utilz;

constexpr long no_edge_value() {
  return ((std::numeric_limits<long>::max)() / 2) - 1;
};

void _impl(square_shape<long> &shape) {
  for (size_t k = 0; k < shape.s(); ++k) {
    long *k_row = shape[k];

    for (size_t i = 0; i < shape.s(); ++i) {
      long *i_row = shape[i];
      long *k_row_l = k_row;

      long ik = i_row[k];
      for (size_t j = 0; j < shape.s(); ++j, ++i_row, ++k_row_l) {
        long distance = ik + *k_row_l;
        if (*i_row > distance)
          *i_row = distance;
      };
    };
  };
};

int main(int argc, char *argv[]) {
  // if (argc < 2) {
  //     std::cerr << "Usage: " << argv[0] << ""
  //         << "Options:\n"
  //         << "\t-h,--help\t\tShow this help message\n"
  //         << "\t-d,--destination DESTINATION\tSpecify the destination path"
  //         << std::endl;
  // }
  size_t matrix_sz = 7;

  long *matrix = (long *)malloc(matrix_sz * matrix_sz * sizeof(long));
  if (matrix == nullptr) {
    std::cerr << "erro: can't allocate memory to hold input matrix";
    return 1;
  }

  // Create rectangular shape
  square_shape<long> shape(matrix, matrix_sz);

  // Fill it with default values
  std::fill(shape.begin(), shape.end(), no_edge_value());

  shape[1][2] = 9;
  shape[1][3] = 2;
  shape[1][6] = 5;
  shape[3][4] = 3;
  shape[3][6] = 6;
  shape[4][6] = 4;
  shape[4][2] = 1;

  _impl(shape);

  for (size_t i = 0; i < shape.s(); ++i)
    for (size_t j = 0; j < shape.s(); ++j)
      if (shape[i][j] != no_edge_value())
        std::cout << "m[" << i << "," << j << "] = " << shape[i][j] << endl;

  return 0;
}

#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>

#include "../../utilz/square_shape.h"

using namespace utilz;

constexpr long
no_edge_value()
{
  return ((std::numeric_limits<long>::max)() / 2) - 1;
};

void
_impl(square_shape<long>& shape)
{
  for (size_t k = 0; k < shape.s(); ++k) {
    long* k_row = shape[k];

    for (size_t i = 0; i < shape.s(); ++i) {
      long* i_row = shape[i];
      long* k_row_l = k_row;

      long ik = i_row[k];
      for (size_t j = 0; j < shape.s(); ++j, ++i_row, ++k_row_l) {
        long distance = ik + *k_row_l;
        if (*i_row > distance)
          *i_row = distance;
      };
    };
  };
};

struct graph_out
{
  square_shape<long> shape;

  void prep(const size_t& vertex_count, const size_t& edge_count)
  {
    long* p = (long*)malloc(vertex_count * vertex_count * sizeof(long));
    if (p == nullptr) {
      throw std::runtime_error("erro: can't allocate memory to hold input matrix");
    }
    shape = square_shape<long>(p, vertex_count);

    std::fill(shape.begin(), shape.end(), no_edge_value());
  }
  void write(const size_t i, const size_t j, const long& v)
  {
    range_check_set<long>(shape, i, j, v);
  }
};

int
main(int argc, char* argv[])
{
  // if (argc < 2) {
  //     std::cerr << "Usage: " << argv[0] << ""
  //         << "Options:\n"
  //         << "\t-h,--help\t\tShow this help message\n"
  //         << "\t-d,--destination DESTINATION\tSpecify the destination path"
  //         << std::endl;
  // }

  std::array<std::tuple<long, long, long>, 7> input = {
    make_tuple(1, 2, 9),
    make_tuple(1, 3, 2),
    make_tuple(1, 6, 5),
    make_tuple(3, 4, 3),
    make_tuple(3, 6, 6),
    make_tuple(4, 6, 4),
    make_tuple(4, 2, 1)
  };

  graph_out out;
  out.prep(7, 7);
  std::for_each(input.begin(), input.end(), [&out](auto &input) -> void {
    long v;
    size_t i, j;

    std::tie(i, j, v) = input;

    out.write(i, j, v);
  });

  square_shape<long> shape = out.shape;

  _impl(shape);

  for (size_t i = 0; i < shape.s(); ++i)
    for (size_t j = 0; j < shape.s(); ++j)
      if (shape[i][j] != no_edge_value())
        std::cout << "m[" << i << "," << j << "] = " << shape[i][j] << endl;

  return 0;
}

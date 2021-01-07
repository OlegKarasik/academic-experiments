#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <tuple>

#include "../../utilz/matrix_io.h"

#include "../../utilz/rect_shape.h"
#include "../../utilz/rect_shape_matrix_io.h"

using namespace utilz;

using matrix_memory           = rect_shape_matrix_memory<long>;
using matrix_output           = rect_shape_matrix_output<long, matrix_memory>;
using matrix_output_predicate = matrix_all_output_predicate<long>;
using matrix_input            = rect_shape_matrix_input<long>;
using matrix_input_predicate  = matrix_except_input_predicate<long>;

constexpr long
no_edge_value()
{
  return ((std::numeric_limits<long>::max)() / 2) - 1;
};

void
calculate(rect_shape<long>& shape)
{
  for (size_t k = 0; k < shape.h(); ++k) {
    long* k_row = shape(k);

    for (size_t i = 0; i < shape.h(); ++i) {
      long* i_row   = shape(i);
      long* k_row_l = k_row;

      long ik = i_row[k];
      for (size_t j = 0; j < shape.w(); ++j, ++i_row, ++k_row_l) {
        long distance = ik + *k_row_l;
        if (*i_row > distance)
          *i_row = distance;
      };
    };
  };
};

int
main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << "\n"
              << "Options:\n"
              << "\t-i,--input\t\tFull path to input file in dimacs9 format\n"
              << "\t-o,--output\t\tFull path to output file in dimacs9 format"
              << std::endl;
    return 1;
  }

  std::string input_path, output_path;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-i" || arg == "--input") {
      if (i == (argc - 1)) {
        std::cerr << "No path has been specified for '-i' command" << std::endl;
        return 1;
      }
      input_path = argv[i + 1];
    }
    if (arg == "-o" || arg == "--output") {
      if (i == (argc - 1)) {
        std::cerr << "No path has been specified for '-o' command" << std::endl;
        return 1;
      }
      output_path = argv[i + 1];
    }
  }

  matrix_memory memory(no_edge_value());

  {
    matrix_output           output(memory);
    matrix_output_predicate predicate;

    fscan_matrix<long>(input_path, predicate, output);
  }

  rect_shape<long> shape = memory();

  is_square_shape(shape);
  calculate(shape);

  {
    matrix_input           input(shape);
    matrix_input_predicate predicate(no_edge_value());

    fprint_matrix<long>(output_path, predicate, input);
  }

  return 0;
}

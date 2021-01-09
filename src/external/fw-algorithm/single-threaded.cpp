#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <tuple>

#include "../../utilz/matrix_io.h"

#include "../../utilz/rect_shape.h"
#include "../../utilz/rect_shape_matrix_io.h"

using namespace utilz;

/* Constant value which indicates that there is no path between two vertexes.
     Please note: this value can be used ONLY when input paths are >= 0.
 */
constexpr long
no_path_value()
{
  return ((std::numeric_limits<long>::max)() / 2) - 1;
};

template<typename T, T V>
class rect_shape_precondition
{
private:
  const T m_s = V;

public:
  rect_shape_precondition()
  {}

  bool operator()(const rect_shape<T>& s)
  {
    for (size_t i = 0; i < s.w(); ++i)
      for (size_t j = 0; j < s.h(); ++j)
        if (s(i, j) < this->m_s)
          return false;

    return s.w() == s.h();
  }
};

using matrix_precondition     = rect_shape_precondition<long, 0>;
using matrix_memory           = rect_shape_matrix_memory<long, no_path_value()>;
using matrix_output           = rect_shape_matrix_output<long, matrix_memory>;
using matrix_output_predicate = matrix_all_predicate<long>;
using matrix_input            = rect_shape_matrix_input<long>;
using matrix_input_predicate  = matrix_except_predicate<long, no_path_value()>;

void
calculate(rect_shape<long>& matrix)
{
  for (size_t k = 0; k < matrix.h(); ++k) {
    long* k_row = matrix(k);

    for (size_t i = 0; i < matrix.h(); ++i) {
      long* i_row   = matrix(i);
      long* k_row_l = k_row;

      long ik = i_row[k];
      for (size_t j = 0; j < matrix.w(); ++j, ++i_row, ++k_row_l) {
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

  // Load matrix from a file. All memory management is handed by "memory" object
  // (which will deallocate memory on destruction).
  matrix_memory memory;

  fscan_matrix<long>(input_path, matrix_output_predicate(), matrix_output(memory));

  rect_shape<long> matrix = memory();

  // Ensure loaded matrix is a square matrix and every cell contains positive value
  matrix_precondition precondition;
  if (!precondition(matrix)) {
    std::cerr << "Input should be a square matrix with values greater or equal to zero" << std::endl;
    return 1;
  }

  // Calculate all shortest paths
  calculate(matrix);

  // Print updated matrix to a file
  fprint_matrix<long>(output_path, matrix_input_predicate(), matrix_input(matrix));

  return 0;
}

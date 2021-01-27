#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>

#include "program.h"

#include "../../utilz/graphs-generators.hpp"
#include "../../utilz/graphs-fio.hpp"
#include "../../utilz/graphs-cio.hpp"

#include "../../utilz/square-shape-io.hpp"
#include "../../utilz/square-shape.hpp"

using namespace utilz;

// Constant value which indicates that there is no path between two vertexes.
// Please note: this value can be used ONLY when input paths are >= 0.
//
constexpr long
no_path_value()
{
  return ((std::numeric_limits<long>::max)() / 2) - 1;
};

// A precondition for input matrix to ensure proper algorithm execution.
// Ensures: matrix is a square matrix; all cell values are greater or equal to zero.
//
// template<typename T, T V>
// class rect_shape_precondition
// {
// private:
//   const T m_s = V;

// public:
//   rect_shape_precondition()
//   {}

//   bool operator()(const rect_shape<T>& s)
//   {
//     for (size_t i = 0; i < s.w(); ++i)
//       for (size_t j = 0; j < s.h(); ++j)
//         if (s(i, j) < this->m_s)
//           return false;

//     return s.w() == s.h();
//   }
// };

// using matrix_precondition     = rect_shape_precondition<long, 0>;
// using matrix_memory           = rect_shape_matrix_memory<long, no_path_value()>;
// using matrix_output           = rect_shape_matrix_output<long, matrix_memory>;
// using matrix_output_predicate = matrix_all_predicate<long>;
// using matrix_input            = rect_shape_matrix_input<long>;
// using matrix_input_predicate  = matrix_except_predicate<long, no_path_value()>;

template<typename T, T V>
class except_predicate
{
private:
  const T m_v = V;

public:
  except_predicate()
  {}

  bool operator()(const T& v) const
  {
    return v != this->m_v;
  }
};

int
main(int argc, char* argv[])
{
  square_shape_memory<size_t, no_path_value()>                           memory;
  square_shape_out<size_t, square_shape_memory<size_t, no_path_value()>> out(memory);

  graphs::io::fscan_graph<size_t, square_shape_out<size_t, square_shape_memory<size_t, no_path_value()>>>("D:/GitHub/academic-experiments/data/direct-acyclic-graphs/7-7.dimacs9", out);

except_predicate<size_t, no_path_value()> p;
square_shape<size_t> s = (square_shape<size_t>)memory;
  square_shape_in<size_t, except_predicate<size_t, no_path_value()>> inzzz(s, p);

  square_shape_memory<size_t, no_path_value()>                           memory2;
  square_shape_out<size_t, square_shape_memory<size_t, no_path_value()>> out2(memory2);

  graphs::random_weighted_directed_acyclic_graph<size_t, square_shape_out<size_t, square_shape_memory<size_t, no_path_value()>>>(10, 10, 1, 2, out2);

square_shape<size_t> s2 = (square_shape<size_t>)memory2;
  square_shape_in<size_t, except_predicate<size_t, no_path_value()>> inzzz2(s2, p);

  graphs::io::fprint_graph<size_t, square_shape_in<size_t, except_predicate<size_t, no_path_value()>>>("D:/GitHub/academic-experiments/data/direct-acyclic-graphs/7-7.dimacs9.out", inzzz);
  graphs::io::cprint_graph<size_t, square_shape_in<size_t, except_predicate<size_t, no_path_value()>>>(inzzz2);

  return 1;
  // if (argc < 3) {
  //   std::cerr << "Usage: " << argv[0] << "\n"
  //             << "Options:\n"
  //             << "\t-i,--input\t\tFull path to input file in dimacs9 format\n"
  //             << "\t-o,--output\t\tFull path to output file in dimacs9 format"
  //             << std::endl;
  //   return 1;
  // }

  // std::string input_path, output_path;
  // for (int i = 1; i < argc; ++i) {
  //   std::string arg = argv[i];
  //   if (arg == "-i" || arg == "--input") {
  //     if (i == (argc - 1)) {
  //       std::cerr << "No path has been specified for '-i' command" << std::endl;
  //       return 1;
  //     }
  //     input_path = argv[i + 1];
  //   }
  //   if (arg == "-o" || arg == "--output") {
  //     if (i == (argc - 1)) {
  //       std::cerr << "No path has been specified for '-o' command" << std::endl;
  //       return 1;
  //     }
  //     output_path = argv[i + 1];
  //   }
  // }

  // graphs::random_weighted_directed_acyclic_graph<long, matrix_output_predicate>(10, 5, 1, 2, matrix_output_predicate());

  // // Load matrix from a file. All memory management is handed by "memory" object
  // // (which will deallocate memory on destruction).
  // //
  // matrix_memory memory;

  // fscan_matrix<long>(input_path, matrix_output_predicate(), matrix_output(memory));

  // rect_shape<long> matrix = memory();

  // // Ensure input matrix matches algorithm precondition
  // //
  // matrix_precondition precondition;
  // if (!precondition(matrix)) {
  //   std::cerr << "Input should be a square matrix with values greater or equal to zero" << std::endl;
  //   return 1;
  // }

  // // Calculate all shortest paths and measure execution time
  // //
  // auto wc_start = std::chrono::high_resolution_clock::now();
  // for (auto i = 0; i < 1000; ++i)
  //   o2_impl(matrix);

  // auto wc_stop     = std::chrono::high_resolution_clock::now();
  // auto wc_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(wc_stop - wc_start);

  // // Print calculation time to standard output
  // //
  // std::cout << wc_duration.count() << std::endl;

  // // Print updated matrix to a file
  // //
  // fprint_matrix<long>(output_path, matrix_input_predicate(), matrix_input(matrix));

  // return 0;
}

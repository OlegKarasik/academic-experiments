#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>

#include "algorithms.hpp"

#include "graphs-io.hpp"

#include "square-shape.hpp"

// Constant value which indicates that there is no path between two vertexes.
// Please note: this value can be used ONLY when input paths are >= 0.
//
constexpr int
no_path_value()
{
  return ((std::numeric_limits<int>::max)() / 2) - 1;
};

using matrix          = utilz::square_shape<int>;
using matrix_sz       = typename utilz::traits::square_shape_traits<matrix>::size_type;
using matrix_set_size = utilz::procedures::square_shape_set_size<matrix>;
using matrix_set_at   = utilz::procedures::square_shape_set_at<matrix>;
using matrix_get_size = utilz::procedures::square_shape_get_size<matrix>;
using matrix_get_at   = utilz::procedures::square_shape_get_at<matrix>;

int
main(int argc, char* argv[]) noexcept
{
  if (!std::cin) {
    std::cerr << "erro: no input; expected a stream which represents "
                 "a matrix in the following format: "
                 "<size> / <from> <to> <count> ... <from> <to> <count>";
    return 1;
  }

  matrix m;

  matrix_set_size set_sz;
  matrix_set_at   set_at;

  auto r_start = std::chrono::high_resolution_clock::now();

  utilz::graphs::io::scan_matrix(std::cin, m, set_sz, set_at);

  auto r_stop     = std::chrono::high_resolution_clock::now();
  auto r_duration = std::chrono::duration_cast<std::chrono::milliseconds>(r_stop - r_start);

  // Replace zeroes in matrix with 'no_path_value'
  //
  for (matrix_sz i = matrix_sz(0); i < m.size(); ++i)
    for (matrix_sz j = matrix_sz(0); j < m.size(); ++j)
      if (m.at(i, j) == 0)
        m.at(i, j) = no_path_value();

  auto w_start = std::chrono::high_resolution_clock::now();

  calculate_apsp(m);

  auto w_stop     = std::chrono::high_resolution_clock::now();
  auto w_duration = std::chrono::duration_cast<std::chrono::milliseconds>(w_stop - w_start);

  std::cerr << "Scan:\t\t" << r_duration.count() << "ms\n"
            << "Execution:\t" << w_duration.count() << "ms" << std::endl;

  // Replace 'no_path_value' with zeroes to reduce output
  //
  for (matrix_sz i = matrix_sz(0); i < m.size(); ++i)
    for (matrix_sz j = matrix_sz(0); j < m.size(); ++j)
      if (m.at(i, j) == no_path_value())
        m.at(i, j) = 0;

  // Output all matrix values
  //
  matrix_get_size get_sz;
  matrix_get_at   get_at;

  utilz::graphs::io::print_matrix(std::cout, m, get_sz, get_at);
}

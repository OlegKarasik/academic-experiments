#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>

#include "algorithms.hpp"

#include "graphs-io.hpp"

#include "square-shape.hpp"

#include "../utilz/io.hpp"

namespace lutilz = ::fw::utilz;
namespace gutilz = ::utilz;

using matrix          = gutilz::square_shape<int>;
using matrix_set_size = gutilz::procedures::square_shape_set_size<matrix>;
using matrix_get_size = gutilz::procedures::square_shape_get_size<matrix>;

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

  auto r_start = std::chrono::high_resolution_clock::now();

  lutilz::io::scan_matrix(std::cin, m, set_sz);

  auto r_stop     = std::chrono::high_resolution_clock::now();
  auto r_duration = std::chrono::duration_cast<std::chrono::milliseconds>(r_stop - r_start);

  auto w_start = std::chrono::high_resolution_clock::now();

  calculate_apsp(m);

  auto w_stop     = std::chrono::high_resolution_clock::now();
  auto w_duration = std::chrono::duration_cast<std::chrono::milliseconds>(w_stop - w_start);

  std::cerr << "Scan:\t\t" << r_duration.count() << "ms\n"
            << "Execution:\t" << w_duration.count() << "ms" << std::endl;

  // Output all matrix values
  //
  matrix_get_size get_sz;

  lutilz::io::print_matrix(std::cout, m, get_sz);
}

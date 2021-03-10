#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>

// global utilz
#include "measure.hpp"
#include "square-shape.hpp"

// local utilz
#include "../utilz/io.hpp"

// algorithm
#include "algorithms.hpp"

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

  std::cerr
    << "Scan: "
    << gutilz::measure_milliseconds([&m]() -> void { lutilz::io::scan_matrix(std::cin, m); })
    << "ms" << std::endl;

  std::cerr
    << "Exec: "
    << gutilz::measure_milliseconds([&m]() -> void { calculate_apsp(m); })
    << "ms" << std::endl;

  std::cerr << "Prnt"
    << gutilz::measure_milliseconds([&m]() -> void { lutilz::io::print_matrix(std::cout, m); })
    << "ms" << std::endl;
}

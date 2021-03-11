#include <iostream>

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
  matrix m;

  auto scan_ms = gutilz::measure_milliseconds([&m]() -> void { lutilz::io::scan_matrix(std::cin, m); });
  std::cerr << "Scan: " << scan_ms << "ms" << std::endl;

  auto exec_ms = gutilz::measure_milliseconds([&m]() -> void { calculate_apsp(m); });
  std::cerr << "Exec: " << exec_ms << "ms" << std::endl;

  auto prnt_ms = gutilz::measure_milliseconds([&m]() -> void { lutilz::io::print_matrix(std::cout, m); });
  std::cerr << "Prnt: " << prnt_ms << "ms" << std::endl;
}

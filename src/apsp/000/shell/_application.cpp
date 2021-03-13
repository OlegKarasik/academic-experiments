#include <iostream>

// global utilz
#include "measure.hpp"
#include "square-shape.hpp"

// local utilz
#include "../../io.hpp"

// algorithm
#if (ALG_VARIATION == 0)
  #include "../00.hpp"
#endif

#if (ALG_VARIATION == 1)
  #include "../01.hpp"
#endif

namespace apsp  = ::apsp;
namespace utilz = ::utilz;

using matrix          = utilz::square_shape<int>;
using matrix_set_size = utilz::procedures::square_shape_set_size<matrix>;
using matrix_get_size = utilz::procedures::square_shape_get_size<matrix>;

int
main(int argc, char* argv[]) noexcept
{
  matrix m;

  auto scan_ms = utilz::measure_milliseconds([&m]() -> void { apsp::io::scan_matrix(std::cin, m); });
  std::cerr << "Scan: " << scan_ms << "ms" << std::endl;

  auto exec_ms = utilz::measure_milliseconds([&m]() -> void { calculate_apsp(m); });
  std::cerr << "Exec: " << exec_ms << "ms" << std::endl;

  auto prnt_ms = utilz::measure_milliseconds([&m]() -> void { apsp::io::print_matrix(std::cout, m); });
  std::cerr << "Prnt: " << prnt_ms << "ms" << std::endl;
}

// global algorithm
//
#if (ALG_VARIATION == 0)
  #include "../algorithms/00.hpp"
#endif

#if (ALG_VARIATION == 1)
  #include "../algorithms/01.hpp"
#endif

// global includes
//
#include <iostream>
#include <memory>

// global utilz
//
#include "measure.hpp"

// local utilz
//
#include "../io.hpp"

// define global types
using g_calculation_type = int;
using g_allocator_type   = std::allocator<g_calculation_type>;

// aliasing
//
#ifdef APSP_ALG_BLOCKED
using matrix = ::utilz::square_shape<::utilz::square_shape<g_calculation_type, g_allocator_type>>;
#else
using matrix = ::utilz::square_shape<g_calculation_type, g_allocator_type>;
#endif

int
main(int argc, char* argv[]) noexcept
{
#ifdef APSP_ALG_BLOCKED
  matrix::size_type s;
  if (argc > 1) {
    s = atoi(argv[1]);
  } else {
    std::cout << "Please specify a block size" << std::endl;
    return 1;
  }
#endif

  // Define matrix and execute algorithm specific overloads of methods
  //
  matrix m;

#ifdef APSP_ALG_BLOCKED
  auto scan_ms = utilz::measure_milliseconds([&m, s]() -> void { ::apsp::io::scan_matrix(std::cin, m, s); });
#else
  auto scan_ms = utilz::measure_milliseconds([&m]() -> void { ::apsp::io::scan_matrix(std::cin, m); });
#endif
  std::cerr << "Scan: " << scan_ms << "ms" << std::endl;

  auto exec_ms = utilz::measure_milliseconds([&m]() -> void { calculate_apsp(m); });
  std::cerr << "Exec: " << exec_ms << "ms" << std::endl;

  auto prnt_ms = utilz::measure_milliseconds([&m]() -> void { ::apsp::io::print_matrix(std::cout, m); });
  std::cerr << "Prnt: " << prnt_ms << "ms" << std::endl;
}

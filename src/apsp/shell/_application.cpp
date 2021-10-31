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
#include <fstream>
#include <iostream>
#include <memory>

// global C includes
//
#include <unistd.h>

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
  std::ifstream ins;
  std::ofstream outs;

#ifdef APSP_ALG_BLOCKED
  matrix::size_type s;

  const char* options = "i:o:b:";
#else
  const char* options = "i:o:";
#endif

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'i':
        ins = std::ifstream(optarg);
        break;
      case 'o':
        outs = std::ofstream(optarg);
        break;
#ifdef APSP_ALG_BLOCKED
      case 'b':
        s = atoi(optarg);
        break;
#endif
    }
  }

  std::istream& in  = ins.is_open() ? ins : std::cin;
  std::ostream& out = outs.is_open() ? outs : std::cout;

  // Define matrix and execute algorithm specific overloads of methods
  //
  matrix m;

#ifdef APSP_ALG_BLOCKED
  auto scan_ms = utilz::measure_milliseconds([&m, &in, s]() -> void { ::apsp::io::scan_matrix(in, m, s); });
#else
  auto scan_ms = utilz::measure_milliseconds([&m, &in]() -> void { ::apsp::io::scan_matrix(in, m); });
#endif

  std::cerr << "Scan: " << scan_ms << "ms" << std::endl;

  auto exec_ms = utilz::measure_milliseconds([&m]() -> void { calculate_apsp(m); });
  std::cerr << "Exec: " << exec_ms << "ms" << std::endl;

  auto prnt_ms = utilz::measure_milliseconds([&m, &out]() -> void { ::apsp::io::print_matrix(out, m); });
  std::cerr << "Prnt: " << prnt_ms << "ms" << std::endl;
}

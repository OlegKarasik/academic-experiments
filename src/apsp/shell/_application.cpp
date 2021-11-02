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

#if (ALG_LARGE_PAGES == 1)
  #include "win-memory.hpp"
#endif

// local utilz
//
#include "../io.hpp"

// large pages integration
//
#if (ALG_LARGE_PAGES == 1)
template<typename T>
using g_allocator_type = typename ::utilz::memory::large_pages_allocator<T>;
#else
template<typename T>
using g_allocator_type = typename std::allocator<T>;
#endif

// define global types
//
using g_calculation_type = int;

// aliasing
//
#ifdef APSP_ALG_BLOCKED
using matrix_block = ::utilz::square_shape<g_calculation_type, g_allocator_type<g_calculation_type>>;
using matrix       = ::utilz::square_shape<matrix_block,       g_allocator_type<matrix_block>>;
#else
using matrix = ::utilz::square_shape<g_calculation_type, g_allocator_type<g_calculation_type>>;
#endif

int
main(int argc, char* argv[]) noexcept
{
  bool binary = false;

  std::ifstream ins;
  std::ofstream outs;

#ifdef APSP_ALG_BLOCKED
  matrix::size_type s;

  const char* options = "i:o:s:b";
#else
  const char* options = "i:o:b";
#endif

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'i':
        ins.open(optarg);
        break;
      case 'o':
        outs.open(optarg);
        break;
      case 'b':
        binary = true;
        break;
#ifdef APSP_ALG_BLOCKED
      case 's':
        s = atoi(optarg);
        break;
#endif
    }
  }

  std::istream& in  = ins.is_open() ? ins : std::cin;
  std::ostream& out = outs.is_open() ? outs : std::cout;

#ifdef APSP_ALG_BLOCKED
  if (s == matrix::size_type()) {
    std::cout << "Please use '-s' and specify block size";
    return 1;
  }
#endif

#if (ALG_LARGE_PAGES == 1)
  // Initialize large pages support from application side
  // this might require different actions in different operating systems
  //
  ::utilz::memory::initialize_large_pages();
#endif

  // Define matrix and execute algorithm specific overloads of methods
  //
  matrix m;

#ifdef APSP_ALG_BLOCKED
  auto scan_ms = utilz::measure_milliseconds([&m, &in, s, binary]() -> void { ::apsp::io::scan_matrix(in, binary, m, s); });
#else
  auto scan_ms = utilz::measure_milliseconds([&m, &in, binary]() -> void { ::apsp::io::scan_matrix(in, binary, m); });
#endif

  std::cerr << "Scan: " << scan_ms << "ms" << std::endl;

  auto exec_ms = utilz::measure_milliseconds([&m]() -> void { calculate_apsp(m); });
  std::cerr << "Exec: " << exec_ms << "ms" << std::endl;

  auto prnt_ms = utilz::measure_milliseconds([&m, &out, binary]() -> void { ::apsp::io::print_matrix(out, binary, m); });
  std::cerr << "Prnt: " << prnt_ms << "ms" << std::endl;
}

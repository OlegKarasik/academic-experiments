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
  const int buf_sz = 1024*64;

  char ins_buf[buf_sz];
  char outs_buf[buf_sz];

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
        ins.rdbuf()->pubsetbuf(ins_buf, buf_sz);
        ins.open(optarg);
        break;
      case 'o':
        outs.rdbuf()->pubsetbuf(outs_buf, buf_sz);
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

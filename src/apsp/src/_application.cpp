// portability
#include "portables/hacks/defines.h"

// instrumentation
#ifdef ITT_TRACE
  #include <ittnotify.h>
#endif

// global includes
//
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

// global C includes
//
#include <stdlib.h>
#ifdef _INTEL_COMPILER
  #include <io.h>

  #include "portables/posix/getopt.h"
#else
  #include <unistd.h>
#endif

// global common includes
//
#include "measure.hpp"
#include "memory.hpp"

// operating system level includes, manage if going cross-platform
//
#include "win-memory.hpp"

// local includes
//
#include "algorithm.hpp"
#include "io.hpp"

// define global types
//
using g_calculation_type = int;

template<typename T>
using g_allocator_type = typename ::utilz::memory::buffer_allocator<T>;

// aliasing
//
#ifdef APSP_ALG_HAS_BLOCKS
using matrix_block = ::utilz::square_shape<g_calculation_type, g_allocator_type<g_calculation_type>>;
using matrix       = ::utilz::square_shape<matrix_block, g_allocator_type<matrix_block>>;
#else
using matrix = ::utilz::square_shape<g_calculation_type, g_allocator_type<g_calculation_type>>;
#endif

int
main(int argc, char* argv[]) __hack_noexcept
{
  bool   opt_binary    = false;
  bool   opt_pages     = false;
  size_t opt_reserve   = 0;
  size_t opt_alignment = 0;

  std::string input;
  std::string output;

#ifdef APSP_ALG_HAS_BLOCKS
  matrix::size_type s = 0;

  const char* options = "i:o:bpr:a:s:";
#else
  const char* options = "i:o:bpr:a:";
#endif

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'i':
        std::cerr << "-i: " << optarg << "\n";

        input = optarg;
        break;
      case 'o':
        std::cerr << "-o: " << optarg << "\n";

        output = optarg;
        break;
      case 'b':
        std::cerr << "-b: true\n";

        opt_binary = true;
        break;
      case 'p':
        std::cerr << "-p: true\n";

        opt_pages = true;
        break;
      case 'r':
        std::cerr << "-r: " << optarg << "\n";

        opt_reserve = atoi(optarg) * 1024 * 1024;

        if (opt_reserve == 0) {
          std::cerr << "erro: missing value after '-r' option";
          return 1;
        }
        break;
      case 'a':
        std::cerr << "-a: " << optarg << "\n";

        opt_alignment = atoi(optarg);

        if (opt_alignment == 0 || opt_alignment % 2 != 0) {
          std::cerr << "erro: missing value after '-a' option or value isn't power of 2";
          return 1;
        }
        break;
#ifdef APSP_ALG_HAS_BLOCKS
      case 's':
        std::cerr << "-s: " << optarg << "\n";

        s = atoi(optarg);

        if (s == matrix::size_type()) {
          std::cerr << "erro: missing value after '-s' option";
          return 1;
        }
        break;
#endif
    }
  }

  std::cerr << std::endl;

  std::ifstream ins;
  if (!input.empty()) {
    if (opt_binary) {
      ins.open(input, std::ios_base::binary);
    } else {
      ins.open(input);
    }
  }

  std::ofstream outs;
  if (!output.empty()) {
    if (opt_binary) {
      outs.open(output, std::ios_base::binary);
    } else {
      outs.open(output);
    }
  }

  std::istream& in  = ins.is_open() ? ins : std::cin;
  std::ostream& out = outs.is_open() ? outs : std::cout;

  std::shared_ptr<char> memory;
  if (opt_pages) {
    // Initialize large pages support from application side
    // this might require different actions in different operating systems
    //
    ::utilz::memory::__largepages_init();

    memory = std::shared_ptr<char>(
      reinterpret_cast<char*>(::utilz::memory::__largepages_malloc(opt_reserve)),
      ::utilz::memory::__largepages_free);

  } else {
    memory = std::shared_ptr<char>(
      reinterpret_cast<char*>(::malloc(opt_reserve)),
      free);
  }

  if (memory == nullptr) {
    std::cerr << "erro: can't allocate memory (size: " << opt_reserve << ")" << std::endl;
    return 1;
  }

  ::utilz::memory::buffer_fx                            buffer_fx(memory, opt_reserve, opt_alignment);
  ::utilz::memory::buffer_allocator<matrix::value_type> buffer_allocator(&buffer_fx);

  // Define matrix and execute algorithm specific overloads of methods
  //
  matrix m(buffer_allocator);

#ifdef APSP_ALG_HAS_BLOCKS
  auto scan_ms = utilz::measure_milliseconds(
    [&m, &in, opt_binary, s]() -> void {
      ::apsp::io::scan_matrix(in, opt_binary, m, s);
    });
#else
  auto scan_ms = utilz::measure_milliseconds(
    [&m, &in, opt_binary]() -> void {
      ::apsp::io::scan_matrix(in, opt_binary, m);
    });
#endif

  std::cerr << "Scan: " << scan_ms << "ms" << std::endl;

#ifdef APSP_ALG_HAS_OPTIONS
  // In cases when algorithm requires additional setup (ex. pre-allocated arrays)
  // it can be done in up procedure (and undone in down).
  //
  // It is important to keep in mind that up acts on memory buffer after the
  // matrix has been allocated.
  //
  auto o = up(m, buffer_fx);
#endif

  auto exec_ms = utilz::measure_milliseconds(
#ifdef APSP_ALG_HAS_OPTIONS
    [&m, &o]() -> void {
#else
    [&m]() -> void {
#endif

#ifdef ITT_TRACE
      __itt_domain*        domain      = __itt_domain_create("apsp.shell");
      __itt_string_handle* handle_exec = __itt_string_handle_create("apsp.shell.exec");
      __itt_task_begin(domain, __itt_null, __itt_null, handle_exec);
#endif

#ifdef APSP_ALG_HAS_OPTIONS
      run(m, o);
#else
      run(m);
#endif

#ifdef ITT_TRACE
      __itt_task_end(domain);
#endif
    });

  std::cerr << "Exec: " << exec_ms << "ms" << std::endl;

#ifdef APSP_ALG_HAS_OPTIONS
  down(m, buffer_fx, o);
#endif

  auto prnt_ms = utilz::measure_milliseconds([&m, &out, opt_binary]() -> void { ::apsp::io::print_matrix(out, opt_binary, m); });
  std::cerr << "Prnt: " << prnt_ms << "ms" << std::endl;
}

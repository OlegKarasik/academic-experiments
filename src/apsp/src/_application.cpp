// portability
#include "portables/hacks/defines.h"

// instrumentation
#ifdef ITT_TRACE
  #include <ittnotify.h>
#endif

// global includes
//
#include <filesystem>
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

// local operating system level includes, manage if going cross-platform
//
#include "win-memory.hpp"

// local utilz
//
#include "measure.hpp"
#include "memory.hpp"
#include "graphs-io.hpp"

#include "square-matrix.hpp"
#include "matrix-traits.hpp"
#include "matrix-manip.hpp"
#include "matrix-io.hpp"

// local includes
//
#include "algorithm.hpp"

// define global types
//
using g_calculation_type = int;

template<typename T>
using g_allocator_type = typename utilz::memory::buffer_allocator<T>;

// aliasing
//
#ifdef APSP_ALG_HAS_BLOCKS
using matrix_block = utilz::square_matrix<g_calculation_type, g_allocator_type<g_calculation_type>>;
using matrix       = utilz::square_matrix<matrix_block, g_allocator_type<matrix_block>>;
#else
using matrix = utilz::square_matrix<g_calculation_type, g_allocator_type<g_calculation_type>>;
#endif

int
main(int argc, char* argv[]) __hack_noexcept
{
  utilz::graphs::io::graph_format opt_input_format  = utilz::graphs::io::graph_format::graph_fmt_none;
  utilz::graphs::io::graph_format opt_output_format = utilz::graphs::io::graph_format::graph_fmt_none;

  bool   opt_pages     = false;
  size_t opt_reserve   = size_t(0);
  size_t opt_alignment = size_t(0);

  std::string opt_input;
  std::string opt_output;

#ifdef APSP_ALG_HAS_BLOCKS
  matrix::size_type opt_block_size = matrix::size_type(0);

  const char* options = "g:G:o:O:pr:a:s:";
#else
  const char* options = "g:G:o:O:pr:a:";
#endif

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'g':
        if (opt_input.empty()) {
          std::cerr << "-g: " << optarg << "\n";

          opt_input = optarg;
          break;
        }
        std::cerr << "erro: unexpected '-g' option detected" << '\n';
        return 1;
      case 'G':
        if (opt_input_format == utilz::graphs::io::graph_format::graph_fmt_none) {
          std::cerr << "-G: " << optarg << "\n";

          if (!utilz::graphs::io::parse_graph_format(optarg, opt_input_format)) {
            std::cerr << "erro: invalid graph format has been detected in '-G' option" << '\n';
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-G' option detected" << '\n';
        return 1;
      case 'o':
        if (opt_output.empty()) {
          std::cerr << "-o: " << optarg << "\n";

          opt_output = optarg;
          break;
        }
        std::cerr << "erro: unexpected '-o' option detected" << '\n';
        return 1;
      case 'O':
        if (opt_output_format == utilz::graphs::io::graph_format::graph_fmt_none) {
          std::cerr << "-O: " << optarg << "\n";

          if (!utilz::graphs::io::parse_graph_format(optarg, opt_output_format)) {
            std::cerr << "erro: invalid graph format has been detected in '-O' option" << '\n';
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-O' option detected" << '\n';
        return 1;
      case 'p':
        if (!opt_pages) {
          std::cerr << "-p: true\n";

          opt_pages = true;
          break;
        }
        std::cerr << "erro: unexpected '-p' option detected" << '\n';
        return 1;
      case 'r':
        if (opt_reserve == size_t(0)) {
          std::cerr << "-r: " << optarg << "\n";

          opt_reserve = atoi(optarg) * 1024 * 1024;

          if (opt_reserve == size_t(0)) {
            std::cerr << "erro: missing value after '-r' option";
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-r' option detected" << '\n';
        return 1;
      case 'a':
        if (opt_alignment == size_t(0)) {
          std::cerr << "-a: " << optarg << "\n";

          opt_alignment = atoi(optarg);

          if (opt_alignment == size_t(0) || opt_alignment % size_t(2) != size_t(0)) {
            std::cerr << "erro: missing value after '-a' option or value isn't power of 2";
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-a' option detected" << '\n';
        return 1;
#ifdef APSP_ALG_HAS_BLOCKS
      case 's':
        if (opt_block_size == matrix::size_type(0)) {
          std::cerr << "-s: " << optarg << "\n";

          opt_block_size = atoi(optarg);

          if (opt_block_size == matrix::size_type(0)) {
            std::cerr << "erro: missing value after '-s' option";
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-a' option detected" << '\n';
        return 1;
#endif
    }
  }
  if (opt_input_format == utilz::graphs::io::graph_fmt_none) {
    std::cerr << "erro: the -G parameter is required";
    return 1;
  }
  if (opt_output_format == utilz::graphs::io::graph_fmt_none) {
    std::cerr << "erro: the -O parameter is required";
    return 1;
  }

  // Open the input stream
  //
  std::ifstream input_fstream(opt_input);
  if (!input_fstream.is_open()) {
    std::cerr << "warn: using standard input instead of a file (please use -g option to redirect input to a file)";
  }

  // Open the output stream
  //
  std::ofstream output_fstream(opt_output);
  if (!output_fstream.is_open()) {
    std::cerr << "warn: using standard output instead of a file (please use -o option to redirect output to a file)";
  }

  std::istream& input_stream  = input_fstream.is_open() ? input_fstream : std::cin;
  std::ostream& output_stream = output_fstream.is_open() ? output_fstream : std::cout;

  std::shared_ptr<char> memory;
  if (opt_pages) {
    // Initialize large pages support from application side
    // this might require different actions in different operating systems
    //
    utilz::memory::__largepages_init();

    memory = std::shared_ptr<char>(
      reinterpret_cast<char*>(utilz::memory::__largepages_malloc(opt_reserve)),
      utilz::memory::__largepages_free);

  } else {
    memory = std::shared_ptr<char>(
      reinterpret_cast<char*>(::malloc(opt_reserve)),
      free);
  }

  if (memory == nullptr) {
    std::cerr << "erro: can't allocate memory (size: " << opt_reserve << ")" << std::endl;
    return 1;
  }

  ::memset(memory.get(), 0, opt_reserve);

  utilz::memory::buffer_fx                            buffer_fx(memory, opt_reserve, opt_alignment);
  utilz::memory::buffer_allocator<matrix::value_type> buffer_allocator(&buffer_fx);

  // Define matrix and execute algorithm specific overloads of methods
  //
  matrix m(buffer_allocator);

#ifdef APSP_ALG_HAS_BLOCKS
  auto scan_ms = utilz::measure_milliseconds(
    [&m, &input_stream, opt_input_format, opt_block_size]() -> void {
      utilz::graphs::io::scan_graph(opt_input_format, input_stream, m, opt_block_size);
    });
#else
  auto scan_ms = utilz::measure_milliseconds(
    [&m, &input_stream, opt_input_format]() -> void {
      utilz::graphs::io::scan_graph(opt_input_format, input_stream, m);
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

  auto prnt_ms = utilz::measure_milliseconds([&m, &output_stream, opt_output_format]() -> void {
    utilz::graphs::io::print_graph(opt_output_format, output_stream, m);
  });
  std::cerr << "Prnt: " << prnt_ms << "ms" << std::endl;
}

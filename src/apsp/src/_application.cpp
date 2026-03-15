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
#include <map>
#include <cmath>

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
#ifdef __APPLE__
#include "osx-memory.hpp"
#endif

#ifdef _WIN32
#include "win-memory.hpp"
#endif

#ifdef APSP_STATISTICS
#define ENABLE_SCOPE_MEASUREMENTS
#endif

// local utilz
//
#include "memory.hpp"
#include "measure.hpp"
#include "graphs-io.hpp"

#include "matrix.hpp"
#include "matrix-manip.hpp"
#include "matrix-traits.hpp"
#include "matrix-io.hpp"
#include "matrix-access.hpp"

// local includes
//
#include "_shell_inject.hpp"

using buffer_type      = ::utilz::memory::buffer_fx;

int
main(int argc, char* argv[]) __hack_noexcept
{
  graph_format_type       opt_input_graph_format       = graph_format_type::graph_fmt_none;
  graph_format_type       opt_output_format            = graph_format_type::graph_fmt_none;

  communities_format_type opt_input_communities_format = communities_format_type::communities_fmt_none;

  bool      opt_pages      = false;
  size_t    opt_reserve    = size_t(0);
  size_t    opt_alignment  = size_t(0);
  size_type opt_block_size = size_type(0);

  std::string opt_input_graph;
  std::string opt_input_communities;
  std::string opt_output;

#ifdef APSP_ALG_MATRIX_FLAT
  const char* options = "g:G:o:O:pr:a:";
#endif

#ifdef APSP_ALG_MATRIX_BLOCKS
  const char* options = "g:G:o:O:pr:a:s:";
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  const char* options = "g:G:o:O:pr:a:c:C:";
#endif

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'g':
        if (opt_input_graph.empty()) {
          std::cerr << "-g: " << optarg << "\n";

          opt_input_graph = optarg;
          break;
        }
        std::cerr << "erro: unexpected '-g' option detected" << '\n';
        return 1;
      case 'G':
        if (opt_input_graph_format == graph_format_type::graph_fmt_none) {
          std::cerr << "-G: " << optarg << "\n";

          if (!::utilz::graphs::io::parse_graph_format(optarg, opt_input_graph_format)) {
            std::cerr << "erro: invalid graph format has been detected in '-G' option" << '\n';
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-G' option detected" << '\n';
        return 1;
      case 'c':
        if (opt_input_communities.empty()) {
          std::cerr << "-c: " << optarg << "\n";

          opt_input_communities = optarg;
          break;
        }
        std::cerr << "erro: unexpected '-c' option detected" << '\n';
        return 1;
      case 'C':
        if (opt_input_communities_format == communities_format_type::communities_fmt_none) {
          std::cerr << "-C: " << optarg << "\n";

          if (!::utilz::communities::io::parse_communities_format(optarg, opt_input_communities_format)) {
            std::cerr << "erro: invalid communities format has been detected in '-C' option" << '\n';
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-C' option detected" << '\n';
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
        if (opt_output_format == graph_format_type::graph_fmt_none) {
          std::cerr << "-O: " << optarg << "\n";

          if (!::utilz::graphs::io::parse_graph_format(optarg, opt_output_format)) {
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

          opt_reserve = size_t(atoi(optarg)) * 1024 * 1024;

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
      case 's':
        if (opt_block_size == matrix_type::size_type(0)) {
          std::cerr << "-s: " << optarg << "\n";

          opt_block_size = atoi(optarg);

          if (opt_block_size == matrix_type::size_type(0)) {
            std::cerr << "erro: missing value after '-s' option";
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-s' option detected" << '\n';
        return 1;
      default:
        return 1;
    }
  }

#ifdef APSP_ALG_MATRIX_BLOCKS
  if (opt_block_size == matrix_type::size_type(0)) {
    std::cerr << "erro: the -s parameter is required";
    return 1;
  }
#endif

  if (opt_input_graph_format == graph_format_type::graph_fmt_none) {
    std::cerr << "erro: the -G parameter is required";
    return 1;
  }
  if (opt_output_format == graph_format_type::graph_fmt_none) {
    std::cerr << "erro: the -O parameter is required";
    return 1;
  }

#ifdef APSP_ALG_MATRIX_CLUSTERS
  if (opt_input_communities_format == communities_format_type::communities_fmt_none) {
    std::cerr << "erro: the -C parameter is required";
    return 1;
  }
#endif

  // Open the input stream
  //
  std::ifstream input_graph_fstream(opt_input_graph);
  if (!input_graph_fstream.is_open()) {
    std::cerr << "erro: the -g parameter is required";
    return 1;
  }

#ifdef APSP_ALG_MATRIX_CLUSTERS
  // Open the input communities stream
  //
  std::ifstream input_communities_fstream(opt_input_communities);
  if (!input_communities_fstream.is_open()) {
    std::cerr << "erro: the -c parameter is required";
    return 1;
  }
#endif

  // Open the output stream
  //
  std::ofstream output_fstream(opt_output);
  if (!output_fstream.is_open()) {
    std::cerr << "warn: using standard output instead of a file (please use -o option to redirect output to a file)";
  }

  std::istream& input_graph_stream = input_graph_fstream;

#if defined(APSP_ALG_MATRIX_CLUSTERS)
  std::istream& input_communities_stream = input_communities_fstream;
#endif

  std::ostream& output_stream = output_fstream.is_open() ? output_fstream : std::cout;

  std::shared_ptr<char> memory;
  if (opt_pages) {
    // Initialize large pages support from application side
    // this might require different actions in different operating systems
    //
    ::utilz::memory::__largepages_init();

    memory = std::shared_ptr<char>(
      reinterpret_cast<char*>(::utilz::memory::__largepages_malloc(opt_reserve)), ::utilz::memory::__largepages_free);
  } else {
    memory = std::shared_ptr<char>(
      reinterpret_cast<char*>(::malloc(opt_reserve)), free);
  }

  if (memory == nullptr) {
    std::cerr << "erro: can't allocate memory (size: " << opt_reserve << ")" << std::endl;
    return 1;
  }

  ::memset(memory.get(), 0, opt_reserve);

  buffer_type            buffer_fx(memory, opt_reserve, opt_alignment);

  // Define matrix and execute algorithm specific overloads of methods
  //
  matrix_type            matrix;
  matrix_run_config_type matrix_run_config;
  matrix_clusters_type   matrix_clusters;

  graph_type       graph       = ::utilz::graphs::io::scan_graph<size_type, value_type>(opt_input_graph_format, input_graph_stream);

#ifdef APSP_ALG_MATRIX_CLUSTERS
  communities_type communities = ::utilz::communities::io::scan_communities<size_type>(opt_input_communities_format, input_communities_stream);
#endif

#ifdef APSP_ALG_MATRIX_FLAT
  scan_matrix_params_type scan_matrix_params(buffer_fx, graph);
#endif
#ifdef APSP_ALG_MATRIX_BLOCKS
  scan_matrix_params_type scan_matrix_params(buffer_fx, graph, opt_block_size);
#endif
#ifdef APSP_ALG_MATRIX_CLUSTERS
  scan_matrix_params_type scan_matrix_params(buffer_fx, graph, communities);
#endif

#ifdef APSP_ALG_ACCESS_FLAT
  matrix_params_type matrix_params;
#endif
#ifdef APSP_ALG_ACCESS_BLOCKS
  matrix_params_type matrix_params(opt_block_size);
#endif
#ifdef APSP_ALG_ACCESS_CLUSTERS
  matrix_params_type matrix_params(communities);
#endif

  auto scan_time = int64_t(0);

  scan_time += ::utilz::measure_milliseconds(
    [&matrix, &scan_matrix_params]() -> void {
      scan_init_matrix(matrix, scan_matrix_params);
    });

  matrix_access_type matrix_access(matrix, matrix_params);

  scan_time += ::utilz::measure_milliseconds(
    [&matrix_access, &scan_matrix_params]() -> void {
      scan_set_matrix(matrix_access, scan_matrix_params);
    });

#ifdef APSP_ALG_MATRIX_CLUSTERS
  scan_time += ::utilz::measure_milliseconds(
    [&matrix_clusters, &scan_matrix_params]() -> void {
      scan_matrix_clusters(matrix_clusters, scan_matrix_params);
    });
#endif

  std::cerr << "Scan: " << scan_time << "ms" << std::endl;

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_MATRIX_CLUSTERS_CONFIGURATION
  auto up_clusters_ms = ::utilz::measure_milliseconds([&c]() -> void { up_clusters(c); });

  std::cerr << "U/CU: " << up_clusters_ms << "ms" << std::endl;
  #endif

  auto op_clusters_ms = ::utilz::measure_milliseconds([&matrix_clusters]() -> void { matrix_clusters.optimise(); });

  std::cerr << "U/CO: " << op_clusters_ms << "ms" << std::endl;

  #ifdef APSP_ALG_MATRIX_CLUSTERS_REARRANGEMENTS
  auto up_arrange_ms = ::utilz::measure_milliseconds([&matrix_access, &matrix_clusters]() -> void {
    matrix_arrange_procedure_type matrix_arrange_procedure;
    matrix_arrange_procedure(
      matrix_access,
      matrix_clusters,
      ::utilz::matrices::procedures::matrix_arrangement::matrix_arrangement_forward);
  });

  std::cerr << "U/AR: " << up_arrange_ms << "ms" << std::endl;
  #endif
#endif

#ifdef APSP_ALG_RUN_CONFIGURATION
  // In cases when algorithm requires additional setup (ex. pre-allocated arrays)
  // it can be done in up procedure (and undone in down).
  //
  // It is important to keep in mind that up acts on memory buffer after the
  // matrix has been allocated.
  //
  auto up_ms = ::utilz::measure_milliseconds(
    [&matrix, &matrix_access, &matrix_run_config, &buffer_fx]() -> void {
      up(matrix, matrix_access, matrix_run_config, buffer_fx);
    });

  std::cerr << "U/CF: " << up_ms << "ms" << std::endl;
#endif

  auto exec_ms = utilz::measure_milliseconds(
    [&matrix, &matrix_clusters, &matrix_run_config]() -> void {

#ifdef ITT_TRACE
      __itt_domain*        domain      = __itt_domain_create("apsp.shell");
      __itt_string_handle* handle_exec = __itt_string_handle_create("apsp.shell.exec");
      __itt_task_begin(domain, __itt_null, __itt_null, handle_exec);
#endif

      SHELL_RUN(matrix, matrix_clusters, matrix_run_config);

#ifdef ITT_TRACE
      __itt_task_end(domain);
#endif
    });

  std::cerr << "Exec: " << exec_ms << "ms" << std::endl;

#ifdef APSP_ALG_RUN_CONFIGURATION
  auto down_ms = utilz::measure_milliseconds(
    [&matrix, &matrix_access, &matrix_run_config, &buffer_fx]() -> void {
      down(matrix, matrix_access, matrix_run_config, buffer_fx);
    });

  std::cerr << "D/CF: " << down_ms << "ms" << std::endl;
#endif

#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_MATRIX_CLUSTERS_REARRANGEMENTS
  auto down_arrange_ms = ::utilz::measure_milliseconds([&matrix_access, &matrix_clusters]() -> void {
    matrix_arrange_procedure_type matrix_arrange_procedure;
    matrix_arrange_procedure(
      matrix_access,
      matrix_clusters,
      ::utilz::matrices::procedures::matrix_arrangement::matrix_arrangement_backward);
  });

  std::cerr << "D/AR: " << down_arrange_ms << "ms" << std::endl;
  #endif
#endif

  auto prnt_ms = ::utilz::measure_milliseconds([&matrix_access, &output_stream, opt_output_format]() -> void {
    ::utilz::matrices::io::print_matrix(opt_output_format, output_stream, matrix_access);
  });
  std::cerr << "Prnt: " << prnt_ms << "ms" << std::endl;

#ifdef APSP_STATISTICS
  for (auto k : utilz::auto_measurements) {
    auto total   = std::reduce(k.second.begin(), k.second.end());
    auto average = total / k.second.size();

    std::cerr << std::setw(4) << k.first << ": (Cnt): " << k.second.size() << std::endl;
    std::cerr << std::setw(4) << k.first << ": (Ttl): " << std::chrono::duration_cast<std::chrono::milliseconds>(total) << std::endl;
    std::cerr << std::setw(4) << k.first << ": (Avg): " << std::chrono::duration_cast<std::chrono::milliseconds>(average) << std::endl;
  }
#endif
}

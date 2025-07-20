// portability
#include "portables/hacks/defines.h"

// global includes
//
#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <random>
#include <sstream>
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

#include "graphs-io.hpp"

namespace utzgio = ::utilz::graphs::io;

// Major type definitions
//
using Index = long;
using Value = long;
using Tuple = std::tuple<Index, Index, Value>;

// This is a tiny program which converts graphs to a different formats
//
int
main(int argc, char* argv[])
{
  utzgio::graph_format opt_input_format  = utzgio::graph_format::graph_fmt_none;
  utzgio::graph_format opt_output_format = utzgio::graph_format::graph_fmt_none;

  std::string opt_input;
  std::string opt_output;

  // Supported options
  // g: <path>, path to input file
  // G: <enum>, format of an input file
  //    Supported values:
  //    - 'edgelist'
  //    - 'dimacs'
  //    - 'weightlist'
  //    - 'binary'
  // o: <path>, path to output file
  // O: <enum>, format of an output file
  //    Supported values
  //    - 'edgelist'
  //    - 'dimacs'
  //    - 'weightlist'
  //    - 'binary'
  //
  const char* options = "g:G:o:O:";

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
        if (opt_input_format == utzgio::graph_format::graph_fmt_none) {
          std::cerr << "-G: " << optarg << "\n";

          if (!utzgio::parse_graph_format(optarg, opt_input_format)) {
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
        if (opt_output_format == utzgio::graph_format::graph_fmt_none) {
          std::cerr << "-O: " << optarg << "\n";

          if (!utzgio::parse_graph_format(optarg, opt_output_format)) {
            std::cerr << "erro: invalid graph format has been detected in '-O' option" << '\n';
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-O' option detected" << '\n';
        return 1;
    }
  }
  if (opt_input.empty()) {
    std::cerr << "erro: the -g parameter is required";
    return 1;
  }
  if (opt_output.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }
  if (opt_input_format == utzgio::graph_fmt_none) {
    std::cerr << "erro: the -G parameter is required";
    return 1;
  }
  if (opt_output_format == utzgio::graph_fmt_none) {
    std::cerr << "erro: the -O parameter is required";
    return 1;
  }

  // Open the input stream
  //
  std::ifstream input_stream(opt_input);
  if (!input_stream.is_open()) {
    std::cerr << "erro: can't open graph file (denoted by -g option)";
    return 1;
  }

  // Open the output stream
  //
  std::ofstream output_stream(opt_output);
  if (!output_stream.is_open()) {
    std::cerr << "erro: can't open output file (denoted by -o option)";
    return 1;
  }

  // Scan graph (from one format)
  //
  auto [vc, edges] = utzgio::scan_graph<Index, Value>(opt_input_format, input_stream);

  // Print graph (to another format)
  //
  utzgio::print_graph(opt_output_format, output_stream, vc, edges);

  // Flush output stream
  //
  output_stream.flush();

  return 0;
}

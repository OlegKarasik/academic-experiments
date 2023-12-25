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

// This is a tiny program which converts graphs to a different formats
//
int
main(int argc, char* argv[])
{
  std::string opt_input;
  std::string opt_output;

  utilz::graphs::io::graph_stream_format opt_input_format  = utilz::graphs::io::graph_stream_format::fmt_none;
  utilz::graphs::io::graph_stream_format opt_output_format = utilz::graphs::io::graph_stream_format::fmt_none;

  char opt_type = 'g';

  // Supported options
  // i: <path>,      path to input file
  // o: <path>,      path to output file
  // f: <enum-pair>, a comma separated pair of input and output formats
  //    Supported values:
  //    - 'edgelist'
  //    - 'dimacs'
  //    - 'weightlist'
  //    - 'binary'
  //
  const char* options = "i:o:f:";

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'i':
        std::cerr << "-i: " << optarg << "\n";

        opt_input = optarg;
        break;
      case 'o':
        std::cerr << "-o: " << optarg << "\n";

        opt_output = optarg;
        break;
      case 'f': {
        std::cerr << "-f: " << optarg << "\n";

        std::string       v;
        std::stringstream ss(optarg);

        if (!std::getline(ss, v, ',') || !utilz::graphs::io::parse_graph_stream_format(v, opt_input_format)) {
          std::cerr << "erro: missed or unsupported input format in '-f' option, must be 'edgelist' or 'dimacs";
          return 1;
        }
        if (!std::getline(ss, v, ',') || !utilz::graphs::io::parse_graph_stream_format(v, opt_output_format)) {
          std::cerr << "erro: missed or unsupported output format in '-f' option, must be 'edgelist' or 'dimacs";
          return 1;
        }

        break;
      }
    }
  }

  if (opt_input.empty()) {
    std::cerr << "erro: the -i parameter is required";
    return 1;
  }
  if (opt_output.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }
  if (opt_input_format == utilz::graphs::io::graph_stream_format::fmt_none || opt_output_format == utilz::graphs::io::graph_stream_format::fmt_none) {
    std::cerr << "erro: the -f parameter is required";
    return 1;
  }
  if (opt_type != 'g') {
    std::cerr << "erro: unsupported input type in '-t' option, must be 'g'";
    return 1;
  }

  // We open both files
  //
  std::ifstream input_stream(opt_input);
  if (input_stream.fail()) {
    std::cerr << "erro: can't open input file";
    return 1;
  };

  std::ofstream output_stream(opt_output);
  if (output_stream.fail()) {
    std::cerr << "erro: can't create output file";
    return 1;
  };

  auto igstream = utilz::graphs::io::make_graph_istream<int, int>(input_stream, opt_input_format);
  auto ogstream = utilz::graphs::io::make_graph_ostream<int, int>(output_stream, opt_output_format);

  auto is = igstream.get();
  auto os = ogstream.get();

  utilz::graphs::io::graph_preamble<int> preamble;
  if (is >> preamble) {
    if (!(os << preamble)) {
      std::cerr << "erro: can't write preamble information to output file";
      return 1;
    }
  }

  utilz::graphs::io::graph_edge<int, int> edge;
  while (is >> edge) {
    if (!(os << edge)) {
      std::cerr << "erro: can't write edge information to output file";
      return 1;
    }
  }
  if (is->fail()) {
    std::cerr << "erro: the input file is in the invalid format or incomplete";
    return 1;
  }

  output_stream.flush();
}

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
#include "square-shape.hpp"

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
          std::cerr << "erro: missed or unsupported input format in '-f' option";
          return 1;
        }
        if (!std::getline(ss, v, ',') || !utilz::graphs::io::parse_graph_stream_format(v, opt_output_format)) {
          std::cerr << "erro: missed or unsupported output format in '-f' option";
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

  // Weight matrix
  //
  utilz::square_shape<int> matrix;

  // Shared accessors
  //
  utilz::graphs::io::value_move_function<utilz::square_shape<int>> move_edge_count;

  // Scan accesors
  //
  utilz::procedures::square_shape_set_size<utilz::square_shape<int>> set_vertex_count;
  utilz::procedures::square_shape_set<utilz::square_shape<int>>      set_value;

  utilz::graphs::io::scan_graph(
    input_stream,
    opt_input_format,
    matrix,
    set_vertex_count,
    move_edge_count,
    set_value);

  // Print accesors
  //
  utilz::procedures::square_shape_get_size<utilz::square_shape<int>> get_vertex_count;
  utilz::procedures::square_shape_get<utilz::square_shape<int>>      get_value;

  utilz::graphs::io::print_graph(
    output_stream,
    opt_output_format,
    matrix,
    get_vertex_count,
    move_edge_count,
    get_value);

  return 0;
}

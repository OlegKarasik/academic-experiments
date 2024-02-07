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

using Index = long;
using Value = long;

// This is a tiny program which converts graphs to a different formats
//
int
main(int argc, char* argv[])
{
  utilz::graphs::io::graph_format opt_input_graph_format  = utilz::graphs::io::graph_format::graph_fmt_none;
  utilz::graphs::io::graph_format opt_output_graph_format = utilz::graphs::io::graph_format::graph_fmt_none;

  std::string opt_input_graph;
  std::string opt_output;

  // Supported options
  // g: <path>, [set: a] path to input file
  // G: <enum>, [set: a] format of a graph file
  //    Supported values:
  //    - 'edgelist'
  //    - 'dimacs'
  //    - 'weightlist'
  //    - 'binary'
  // c: <path>, [set: b] path to a communities file
  // C: <enum>, [set: b] format of a communities file
  //    Supported values:
  //    - 'rlang'
  // o: <path>, path to output file
  // O: <enum/enum>, format of a graph output file or format of a communities output file
  //    Supported values (when used with -g/-G):
  //    - 'edgelist'
  //    - 'dimacs'
  //    - 'weightlist'
  //    - 'binary'
  //    Supported values (when used with -c/-C):
  //    - 'rlang'
  //
  const char* options = "g:G:o:O:";

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
        if (opt_input_graph_format == utilz::graphs::io::graph_format::graph_fmt_none) {
          std::cerr << "-G: " << optarg << "\n";

          if (!utilz::graphs::io::parse_graph_format(optarg, opt_input_graph_format)) {
            std::cerr << "erro: invalid graph format has been detected in '-G' option" << '\n';
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-G' option detected" << '\n';
        return 1;
      case 'o':
        std::cerr << "-o: " << optarg << "\n";

        if (opt_output.empty()) {
          opt_output = optarg;
          break;
        }
        std::cerr << "erro: unexpected '-o' option detected" << '\n';
        return 1;
      case 'O':
        if (opt_output_graph_format == utilz::graphs::io::graph_format::graph_fmt_none) {
          std::cerr << "-O: " << optarg << "\n";

          if (!utilz::graphs::io::parse_graph_format(optarg, opt_input_graph_format)) {
            std::cerr << "erro: invalid graph format has been detected in '-O' option" << '\n';
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-O' option detected" << '\n';
        return 1;
    }
  }
  if (opt_output.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }
  if (!opt_input_graph.empty()) {
    if (opt_input_graph_format == utilz::graphs::io::graph_fmt_none) {
      std::cerr << "erro: the -G parameter is required";
      return 1;
    }
    if (opt_output_graph_format == utilz::graphs::io::graph_fmt_none) {
      std::cerr << "erro: the -O parameter must be set to graph format when used with -g and -G options";
      return 1;
    }
  }

  if (!opt_input_graph.empty()) {
    utilz::square_shape<Index> graph_matrix;

    auto set_vc = std::function([](utilz::square_shape<Index>& c, Index vc) -> void {
      utilz::procedures::square_shape_set_size<utilz::square_shape<Index>> set_size;
      set_size(c, vc);
    });
    auto set_ec = std::function([](utilz::square_shape<Index>& c, Index ec) -> void {
    });
    auto set_w  = std::function([](utilz::square_shape<Index>& c, Index f, Index t, Value w) -> void {
      utilz::procedures::square_shape_at<utilz::square_shape<Index>> at;
      at(c, f, t) = w;
    });

    std::ifstream graph_stream(opt_input_graph);
    if (!graph_stream.is_open()) {
      std::cerr << "erro: can't open graph file (denoted by -g option)";
      return 1;
    }

    utilz::graphs::io::scan_graph(opt_input_graph_format, graph_stream, graph_matrix, set_vc, set_ec, set_w);
  }

  return 0;
}

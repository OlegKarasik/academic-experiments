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
  utilz::graphs::io::graph_format opt_input_format  = utilz::graphs::io::graph_format::graph_fmt_none;
  utilz::graphs::io::graph_format opt_output_format = utilz::graphs::io::graph_format::graph_fmt_none;

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

  // Initialise containers for `vertex count` and `edge count` data. Capturing
  // `vertex count` and `edge count` information can optimise convertion
  // in case input and output formats have same requirements for metadata
  //
  Index vc = Index(0), ec = Index(0);

  // Initialise functors to scan graph data (`set_vc` and `set_ec` are called depending
  // on the graph format)
  //
  auto set_vc = std::function([&vc](std::vector<Tuple>& c, Index vertex_count) -> void {
    vc = vertex_count;
  });
  auto set_ec = std::function([&ec](std::vector<Tuple>& c, Index edge_count) -> void {
    ec = edge_count;
  });
  auto set_w  = std::function([](std::vector<Tuple>& c, Index f, Index t, Value w) -> void {
    c.push_back(std::make_tuple(f, t, w));
  });

  // Initialise functors to print graph data (`get_vc` and `get_ec` are called depending
  // on the graph format)
  //
  auto get_vc = std::function([&vc](std::vector<Tuple>& c) -> std::tuple<bool, Index> {
    return std::make_tuple(vc != Index(0), vc);
  });
  auto get_ec = std::function([&ec](std::vector<Tuple>& c) -> std::tuple<bool, Index> {
    return std::make_tuple(ec != Index(0), ec);
  });
  auto get_it = std::function([](std::vector<Tuple>& c) -> std::tuple<std::vector<Tuple>::iterator, std::vector<Tuple>::iterator> {
    return std::make_tuple(c.begin(), c.end());
  });

  // Initialise a container for graph edges
  //
  std::vector<Tuple> graph_edges;

  // Scan graph and capture `vertex count` and `edge count` information if provided
  //
  utilz::graphs::io::scan_graph(
    opt_input_format,
    input_stream,
    graph_edges,
    set_vc,
    set_ec,
    set_w);

  // Print graph and reuse `vertex count` and `edge count` information if provided
  //
  utilz::graphs::io::print_graph(
    opt_output_format,
    output_stream,
    graph_edges,
    get_vc,
    get_ec,
    get_it);

  // Flush output stream
  //
  output_stream.flush();

  return 0;
}

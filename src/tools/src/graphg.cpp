// portability
#include "portables/hacks/defines.h"

// global includes
//
#include <algorithm>
#include <fstream>
#include <iostream>
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

#include "graphs-generators.hpp"
#include "graphs-io.hpp"
#include "square-shape.hpp"

// This is a tiny program which generates random graphs
//
int
main(int argc, char* argv[])
{
  utilz::graphs::io::graph_format opt_output_format = utilz::graphs::io::graph_format::graph_fmt_none;

  std::string opt_output;
  int         opt_algorithm    = -1;
  int         opt_vertex_count = -1;
  int         opt_edge_percent = -1;
  int         opt_low_weight   = 1;
  int         opt_high_weight  = 20;

  // Supported options
  // o: <path>, path to store generated graph
  // f: <enum>, output format
  //    Supported values:
  //    - 'edgelist'
  //    - 'dimacs'
  //    - 'weightlist'
  //    - 'binary'
  // a: <int>,  graph generation algorithm to use
  //    Supported values:
  //    - 0: Random Directed Acyclic Graph
  //    - 1: Random Complete Graph
  //    - 2: Random Connected Graph
  // v: <int>,  number of vertex in a graph
  // e: <int>,  percentage of edges in a graph (based on vertex count)
  //    Supported values:
  //    - 0 to 100 with step 1
  //    Required in algorithms:
  //    - 0: Random Directed Acyclic Graph (DAG / S)
  //    - 2: Random Connected Graph
  // l: <int>,  minimal weight of an edge
  //    Supported values:
  //    - 0 to 'h' with step 1
  // h: <int>,  maximal weight of an edge
  //    Supported values:
  //    - 'l' to any with step 1
  //
  const char* options = "o:a:v:e:l:h:f:";

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'o':
        std::cerr << "-o: " << optarg << "\n";

        opt_output = optarg;
        break;
      case 'f': {
        std::cerr << "-f: " << optarg << "\n";

        if (!utilz::graphs::io::parse_graph_format(optarg, opt_output_format)) {
          std::cerr << "erro: missed or unsupported output format in '-f' option";
          return 1;
        }
        break;
      }
      case 'a':
        std::cerr << "-a: " << optarg << "\n";

        opt_algorithm = atoi(optarg);

        if (opt_algorithm < 0 || opt_algorithm > 2) {
          std::cerr << "erro: unsupported algorithm specified in '-a' option";
          return 1;
        }
        break;
      case 'v':
        std::cerr << "-v: " << optarg << "\n";

        opt_vertex_count = atoi(optarg);

        if (opt_vertex_count <= 0) {
          std::cerr << "erro: unsupported number of vertexes specified in '-v' option";
          return 1;
        }
        break;
      case 'e':
        std::cerr << "-e: " << optarg << "\n";

        opt_edge_percent = atoi(optarg);

        if (opt_edge_percent < 1 || opt_edge_percent > 100) {
          std::cerr << "erro: unsupported percent of edges specified in '-e' option";
          return 1;
        }
        break;
      case 'l':
        std::cerr << "-l: " << optarg << "\n";

        opt_low_weight = atoi(optarg);

        if (opt_low_weight < 1) {
          std::cerr << "erro: unsupported weight specified in '-l' option";
          return 1;
        }
        break;
      case 'h':
        std::cerr << "-h: " << optarg << "\n";

        opt_high_weight = atoi(optarg);

        if (opt_high_weight < 1) {
          std::cerr << "erro: unsupported weight specified in '-h' option";
          return 1;
        }
        break;
    }
  }

  if (opt_output.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }
  if (opt_vertex_count == 0) {
    std::cerr << "erro: the -v parameter is required";
    return 1;
  }
  if (opt_low_weight >= opt_high_weight) {
    std::cerr << "erro: the value of lowest edge weight can't be greater or equal to highest edge weight value";
    return 1;
  }

  std::ofstream outs(opt_output);

  // All graph generators do fill adjacency matrix with edges information
  //
  utilz::square_shape<bool> adjacency_matrix;

  try {
    switch (opt_algorithm) {
      case 0: {
        // Calculate edge count based on requested edge percent
        //
        size_t edge_count = size_t(((opt_vertex_count * (opt_vertex_count - 1)) / 2) * ((float)opt_edge_percent / 100));

        // Random Directed Acyclic Graph (DAG / S)
        //
        adjacency_matrix = utilz::graphs::generators::random_graph(
          opt_vertex_count,
          edge_count,
          utilz::graphs::generators::directed_acyclic_graph_tag());
        break;
      }
      case 1:
        // Random Complete Graph
        //
        adjacency_matrix = utilz::graphs::generators::random_graph(
          opt_vertex_count,
          utilz::graphs::generators::complete_graph_tag());
        break;
      case 2: {
        // Calculate edge count based on requested edge percent
        //
        size_t edge_count = size_t(((opt_vertex_count * (opt_vertex_count - 1)) / 2) * ((float)opt_edge_percent / 100));

        // Random Connected Graph
        //
        adjacency_matrix = utilz::graphs::generators::random_graph(
          opt_vertex_count,
          edge_count,
          utilz::graphs::generators::connected_graph_tag());

        break;
      }
    }
  } catch (const std::logic_error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  // We use uniform distribution to get random weight values
  //
  std::mt19937_64                    weight_distribution_engine;
  std::uniform_int_distribution<int> weight_distribution(opt_low_weight, opt_high_weight);

  weight_distribution_engine.seed(std::chrono::system_clock::now().time_since_epoch().count());

  // Weight matrix
  //
  utilz::square_shape<int> weight_matrix(adjacency_matrix.size());

  // Matrix accesors
  //
  utilz::procedures::square_shape_get_size<utilz::square_shape<int>> get_vertex_count;
  utilz::graphs::io::null_get_function<utilz::square_shape<int>>     get_edge_count;
  utilz::procedures::square_shape_get<utilz::square_shape<int>>      get_value;

  // Update adjacency matrix with weight values, effectively transforming
  // it to representation of directed, weighted graph
  //
  for (auto i = 0; i < adjacency_matrix.size(); ++i)
    for (auto j = 0; j < adjacency_matrix.size(); ++j)
      if (adjacency_matrix.at(i, j))
        weight_matrix.at(i, j) = weight_distribution(weight_distribution_engine);

  // Save
  //
  utilz::graphs::io::print_graph(
    outs,
    opt_output_format,
    weight_matrix,
    get_vertex_count,
    get_edge_count,
    get_value);
}

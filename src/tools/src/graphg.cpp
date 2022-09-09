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

int
main(int argc, char* argv[])
{
  bool opt_binary       = false;
  int  opt_algorithm    = -1;
  int  opt_vertex_count = -1;
  int  opt_edge_percent = -1;
  int  opt_low_weight   = 1;
  int  opt_high_weight  = 20;

  std::ofstream outs;

  // Supported options
  // o: <path>, path to store generated graph
  // b: <flag>, indicates whether graph should be stored in binary or textual representation
  // a: <int>,  graph generation algorithm to use
  //    Supported values:
  //    - 0: Random Directed Acyclic Graph (DAG)
  //    - 1: Random Complete Graph
  // v: <int>,  number of vertex in a graph
  // e: <int>,  percentage of edges in a graph (based on vertex count)
  //    Supported values:
  //    - 0 to 100 with step 1
  //    Required in algorithms:
  //    - 0: Random Directed Acyclic Graph (DAG)
  // l: <int>,  minimal weight of an edge
  //    Supported values:
  //    - 0 to 'h' with step 1
  // h: <int>,  maximal weight of an edge
  //    Supported values:
  //    - 'l' to any with step 1
  //
  const char* options = "o:ba:v:e:l:h:";

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'o':
        std::cerr << "-o: " << optarg << "\n";

        outs.open(optarg);
        break;
      case 'b':
        std::cerr << "-b: true\n";

        opt_binary = true;
        break;
      case 'a':
        std::cerr << "-a: " << optarg << "\n";

        opt_algorithm = atoi(optarg);

        if (opt_algorithm < 0 || opt_algorithm > 1) {
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

  if (!outs.is_open()) {
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

  // All graph generators do fill adjacency matrix with edges information
  //
  utilz::square_shape<int> adjacency_matrix;

  // Matrix accesors
  //
  utilz::procedures::square_shape_set_size<utilz::square_shape<int>> set_size;
  utilz::procedures::square_shape_set<utilz::square_shape<int>>      set_value;
  utilz::procedures::square_shape_get_size<utilz::square_shape<int>> get_size;
  utilz::procedures::square_shape_get<utilz::square_shape<int>>      get_value;

  try {
    switch (opt_algorithm) {
      case 0: {
        // Promised paths to include in a graph (current not implemented from CLI)
        //
        std::vector<utilz::graphs::generators::promised_path<utilz::square_shape<int>::size_type>> promised_paths;

        // Calculate edge count based on requested edge percent
        //
        size_t edge_count = size_t(((opt_vertex_count * (opt_vertex_count - 1)) / 2) * ((float)opt_edge_percent / 100));

        // Random Directed Acyclic Graph (DAG)
        //
        utilz::graphs::generators::random_graph(
          opt_vertex_count,
          edge_count,
          promised_paths,
          adjacency_matrix,
          set_size,
          set_value,
          utilz::graphs::generators::directed_acyclic_graph_tag());
      } break;
      case 1:
        // Random Complete Graph
        //
        utilz::graphs::generators::random_graph(
          opt_vertex_count,
          adjacency_matrix,
          set_size,
          set_value,
          utilz::graphs::generators::complete_graph_tag());
        break;
    }
  } catch (const std::logic_error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  // We use uniform distribution to get random weight values
  //
  std::mt19937_64                    weight_distribution_engine;
  std::uniform_int_distribution<int> weight_distribution(opt_low_weight, opt_high_weight);

  // Update adjacency matrix with weight values, effectively transforming
  // it to representation of directed, weighted graph
  //
  for (auto i = 0; i < adjacency_matrix.size(); ++i)
    for (auto j = 0; j < adjacency_matrix.size(); ++j)
      if (adjacency_matrix.at(i, j) == 1)
        adjacency_matrix.at(i, j) = weight_distribution(weight_distribution_engine);

  // Save
  //
  utilz::graphs::io::print_matrix(outs, opt_binary, adjacency_matrix, get_size, get_value);
}

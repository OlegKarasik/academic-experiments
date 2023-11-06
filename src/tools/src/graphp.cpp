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
#include <chrono>

// global C includes
//
#include <stdlib.h>
#ifdef _INTEL_COMPILER
  #include <io.h>

  #include "portables/posix/getopt.h"
#else
  #include <unistd.h>
#endif

// This is a tiny program that prints input graph in .dot format
//
int
main(int argc, char* argv[])
{
  std::string opt_input_edges;
  std::string opt_input_clusters;
  std::string opt_output_path;

  // Supported options
  // i: <path>, path to input edges (no size, no weights), if specified twice then
  //            first one is interpreted as a path to edges and second as a path to clusters
  // o: <path>, path to output
  //
  const char* options = "i:o:";

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'i':
        std::cerr << "-i: " << optarg << "\n";

        if (opt_input_edges.empty()) {
          opt_input_edges = optarg;
          break;
        }
        if (opt_input_clusters.empty()) {
          opt_input_clusters = optarg;
          break;
        }
        std::cerr << "erro: unexpected '-i' option detected" << '\n';
        return 1;
      case 'o':
        std::cerr << "-o: " << optarg << "\n";

        opt_output_path = optarg;
        break;
    }
  }

  if (opt_input_edges.empty()) {
    std::cerr << "erro: the -i parameter is required";
    return 1;
  }
  if (opt_output_path.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }

  std::ifstream edges_stream(opt_input_edges);
  std::ofstream output_stream(opt_output_path);

  output_stream << "digraph cats {\n";

  // Print edges
  //
  {
    std::string line;
    while (std::getline(edges_stream, line)) {
      std::istringstream iss(line);

      int f, t;
      iss >> f >> t;

      output_stream << "  " << f << " -> " << t << "\n";
    }
  }
  // Print clusters
  //
  {
    if (!opt_input_clusters.empty()) {
      std::ifstream clusters_stream(opt_input_clusters);

      int index = 0;

      std::string line;
      while (std::getline(clusters_stream, line)) {
        output_stream << "  subgraph \"" << index << "\" {\n";
        output_stream << "    cluster=true;\n";

        std::istringstream iss(line);

        int v;
        while (iss >> v) {
          output_stream << "    \"" << v << "\";\n";
        }

        output_stream << "  }\n";

        ++index;
      }
    }
  }

  output_stream << "}";
  output_stream.flush();
}

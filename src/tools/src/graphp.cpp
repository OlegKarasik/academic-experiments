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
  std::string opt_graph_path;
  std::string opt_clusters_path;
  std::string opt_output_path;

  // Supported options
  // g: <path>, path to graph
  // c: <path>, path to clusters
  // o: <path>, path to output
  //
  const char* options = "g:c:o:";

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'g':
        std::cerr << "-g: " << optarg << "\n";

        opt_graph_path = optarg;
        break;
      case 'c':
        std::cerr << "-c: " << optarg << "\n";

        opt_clusters_path = optarg;
        break;
      case 'o':
        std::cerr << "-o: " << optarg << "\n";

        opt_output_path = optarg;
        break;
    }
  }

  if (opt_graph_path.empty()) {
    std::cerr << "erro: the -g parameter is required";
    return 1;
  }
  if (opt_output_path.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }

  std::ifstream graph_stream;
  std::ifstream clusters_stream;
  std::ofstream output_stream;

  graph_stream.open(opt_graph_path);
  output_stream.open(opt_output_path);

  if (!opt_clusters_path.empty()) {
    clusters_stream.open(opt_clusters_path);
  }

  output_stream << "digraph cats {\n";

  {
    std::string line;
    while (std::getline(graph_stream, line)) {
      std::istringstream iss(line);

      int f, t;
      iss >> f >> t;

      output_stream << "  " << f << " -> " << t << "\n";
    }
  }
  {
    if (clusters_stream.is_open()) {
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

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

// This is a tiny program which generates random weight information for an
// unweighted, directed graph represented in form of edges:
// 0 1
// 5 2
// ... and so on
//
// and produces the weighted, directed graph represented in form of edges + weight
// 0 1 5
// 5 2 10
// ... and so on
//
int
main(int argc, char* argv[])
{
  bool opt_print_size  = false;
  int  opt_low_weight  = 1;
  int  opt_high_weight = 20;

  std::string input;
  std::string output;

  // Supported options
  // i: <path>, path to input graph (without weights, only edges)
  // o: <path>, path to output graph (with generated weights)
  // s: <flag>, indicates whether application should output size (number of vertexes) of the graph as the first line
  // l: <int>,  minimal weight of an edge
  //    Supported values:
  //    - 0 to 'h' with step 1
  // h: <int>,  maximal weight of an edge
  //    Supported values:
  //    - 'l' to any with step 1
  //
  const char* options = "i:o:l:h:s";

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'i':
        std::cerr << "-i: " << optarg << "\n";

        input = optarg;
        break;
      case 'o':
        std::cerr << "-o: " << optarg << "\n";

        output = optarg;
        break;
      case 's':
        std::cerr << "-s: true\n";

        opt_print_size = true;
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

  if (input.empty()) {
    std::cerr << "erro: the -i parameter is required";
    return 1;
  }
  if (output.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }
  if (opt_low_weight >= opt_high_weight) {
    std::cerr << "erro: the value of lowest edge weight can't be greater or equal to highest edge weight value";
    return 1;
  }

  std::ifstream inps;
  std::ofstream outs;

  inps.open(input);
  outs.open(output);

  // We use uniform distribution to get random weight values
  //
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

  std::mt19937_64                    weight_distribution_engine(seed);
  std::uniform_int_distribution<int> weight_distribution(opt_low_weight, opt_high_weight);

  // Read (`from vertex` `to vertex`) and write (`from vertex` `to vertex` `weight`)
  //
  int f, t;
  if (opt_print_size) {
    int min = 1, max = 0;
    while (inps >> f >> t) {
      max = std::max({ max, f, t });
      min = std::min({ min, f, t });
    }
    // If we count vertex numbers from zero, then the number of vertexes
    // would be max mentioned vertex + 1
    //
    if (min == 0) max++;

    outs << max << '\n';

    inps.clear();
    inps.seekg(std::ios::beg);
  }

  while (inps >> f >> t) {
    int v = weight_distribution(weight_distribution_engine);

    outs << f << ' ' << t << ' ' << v << '\n';
  }

  outs.flush();
}

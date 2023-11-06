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

// This is a tiny program which can modify the graph or
// clusters described in a simple text format
//
int
main(int argc, char* argv[])
{
  std::string opt_input;
  std::string opt_output;
  char        opt_type        = 'e';
  bool        opt_parse_w     = false;
  bool        opt_parse_s     = false;
  bool        opt_add_w       = false;
  bool        opt_add_s       = false;
  int         opt_delta       = 0;
  int         opt_low_weight  = 0;
  int         opt_high_weight = 0;

  // Supported options
  // i: <path>,   path to input file
  // o: <path>,   path to output file
  // t: <string>, type of input
  //    Supported values:
  //    - 'e' means input is graph edges, depending on the -p w it could have an additional <weight> value:
  //          <from> <to>
  //          <from> <to>
  //          ...
  //    - 'c' means input is a set of clusteres in the following form:
  //          <vertex> <vertex> <vertex>
  //          <vertex> <vertex>
  //          ...
  // d: <int>, a value used to increment or decrement vertex indexes
  // p: <flag-list>, comma separated list of flags to correctly parse the input
  //    Supported values:
  //    - 'w' requires 'e', means that input file includes graph edges with weights
  //    - 's' requires 'e', means that input file starts with number of vertexes in a graph
  // a: <flag-list>, list of flags which affect output:
  //    - 'w' requires 'e', means that output file will have graph edges with weights (either parsed or random)
  //    - 's' requires 'e', means that output file will start with number vertexes in a graph
  // l: <int>, requires 'e', requires '-a w', means minimal random weight of an edge
  //    Supported values:
  //    - 0 to 'h' with step 1
  // h: <int>, requires 'e', requires '-a w', means maximal random weight of an edge
  //    Supported values:
  //    - 'l' to any with step 1
  //
  const char* options = "i:o:t:p:a:r:d:l:h:";

  // Parse <flag-list>
  //
  auto parse = [](const std::string& in, bool& w, bool& s) -> bool {
    for (auto c : in) {
      switch (c) {
        case 'w':
          w = true;
          break;
        case 's':
          s = true;
          break;
        default:
          return false;
      }
    }
    return true;
  };
  auto validate_parse = [](char t, bool w, bool s) -> bool {
    return !((w || s) && t != 'e');
  };

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
      case 't':
        std::cerr << "-t: " << optarg << "\n";

        opt_type = *optarg;
        break;
      case 'p':
        std::cerr << "-p: " << optarg << "\n";

        if (!parse(optarg, opt_parse_w, opt_parse_s)) {
          std::cerr << "erro: unsupported input type in '-p' option, must be 'w', 'c' or their combinations";
          return 1;
        }
        break;
      case 'a':
        std::cerr << "-a: " << optarg << "\n";

        if (!parse(optarg, opt_add_w, opt_add_s)) {
          std::cerr << "erro: unsupported input type in '-p' option, must be 'w', 'c' or their combinations";
          return 1;
        }
        break;
      case 'd':
        std::cerr << "-d: " << optarg << "\n";

        opt_delta = atoi(optarg);
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

  if (opt_input.empty()) {
    std::cerr << "erro: the -i parameter is required";
    return 1;
  }
  if (opt_output.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }
  if (opt_type != 'e' && opt_type != 'c') {
    std::cerr << "erro: unsupported input type in '-t' option, must be 'e' or 'c'";
    return 1;
  }
  if (!opt_parse_w && opt_add_w) {
    if (opt_low_weight >= opt_high_weight) {
      std::cerr << "erro: the value of lowest edge weight '-l' can't be greater or equal to highest edge weight '-h'";
      return 1;
    }
  }
  if (!validate_parse(opt_type, opt_parse_w, opt_parse_s)) {
    std::cerr << "erro: unsupported combination of '-p' and '-t'";
    return 1;
  }
  if (!validate_parse(opt_type, opt_parse_w, opt_parse_s)) {
    std::cerr << "erro: unsupported combination of '-a' and '-t'";
    return 1;
  }

  // We use uniform distribution to get random weight values
  //
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

  std::mt19937_64                    w_dist_engine(seed);
  std::uniform_int_distribution<int> w_dist(opt_low_weight, opt_high_weight);

  // We open both files
  //
  std::ifstream input_stream(opt_input);
  std::ofstream output_stream(opt_output);

  switch (opt_type) {
    case 'e': {
      // Step 1: process 's' argument on both '-p' and '-a' options
      //
      switch ((opt_parse_s << 1 | opt_add_s)) {
        case 2: { // -p
          int v;
          input_stream >> v;
          break;
        }
        case 1: { // -a
          int min = 1, max = 0;

          int f, t, w;
          while (input_stream >> f >> t) {
            if (opt_parse_w) {
              input_stream >> w;
            }
            max = std::max({ max, f, t });
            min = std::min({ min, f, t });
          }
          // If we count vertex numbers from zero, then the number of vertexes
          // would be max mentioned vertex + 1
          //
          if (min == 0)
            max++;

          output_stream << max << '\n';

          input_stream.clear();
          input_stream.seekg(std::ios::beg);
          break;
        }
        case 3: { // -p & -a
          int v;
          input_stream >> v;
          output_stream << v << '\n';
          break;
        }
      }

      // Step 2: process edges taking
      //
      int f, t, w;
      while (input_stream >> f >> t) {
        output_stream << (f + opt_delta) << ' ' << (t + opt_delta);

        switch (opt_parse_w << 1 | opt_add_w) {
          case 1: { // -a
            output_stream << ' ' << w_dist(w_dist_engine);
            break;
          }
          case 2: { // -p
            input_stream >> w;
            break;
          }
          case 3: { // -p & -a
            input_stream >> w;
            output_stream << ' ' << w;
            break;
          }
        }

        output_stream << '\n';
      }
      break;
    }
    case 'c': {
      std::string line;
      while (std::getline(input_stream, line)) {
        int v;

        std::istringstream iss(line);
        while (iss >> v) {
          output_stream << (v + opt_delta) << ' ';
        }

        output_stream << '\n';
      }
      break;
    }
  }

  output_stream.flush();
}

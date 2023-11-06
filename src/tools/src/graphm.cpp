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

struct parameters
{
  int                                 delta;
  std::mt19937_64&                    distribution_engine;
  std::uniform_int_distribution<int>& distribution;
};

struct context
{
  int f;
  int t;
  int w;
};

bool
read_ft(std::istream& in, context& ctx, parameters params)
{
  return (bool)(in >> ctx.f >> ctx.t);
};

bool
read_ftw(std::istream& in, context& ctx, parameters params)
{
  return (bool)(in >> ctx.f >> ctx.t >> ctx.w);
};

bool
read_ftr(std::istream& in, context& ctx, parameters params)
{
  ctx.w = params.distribution(params.distribution_engine);
  return (bool)(in >> ctx.f >> ctx.t);
};

bool
write_ft(std::ostream& out, context& ctx, parameters params)
{
  return (bool)(out << (ctx.f + params.delta) << ' ' << (ctx.t + params.delta) << ' ' << '\n');
};

bool
write_ftw(std::ostream& out, context& ctx, parameters params)
{
  return (bool)(out << (ctx.f + params.delta) << ' ' << (ctx.t + params.delta) << ' ' << ctx.w << '\n');
};

// This is a tiny program which can modify the graph or
// clusters described in a simple text format
//
int
main(int argc, char* argv[])
{
  std::string opt_input;
  std::string opt_output;
  char        opt_type        = 'e';
  std::string opt_parse       = "";
  std::string opt_add         = "";
  int         opt_delta       = 0;
  int         opt_low_weight  = 1;
  int         opt_high_weight = 20;

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
  // d: <number>, a value used to increment or decrement vertex indexes
  // p: <flag-list>, comma separated list of flags to correctly parse the input
  //    Supported values:
  //    - 'w' requires 'e', means that input file includes graph edges with weights
  //    - 's' requires 'e', means that input file starts with number of vertexes in a graph
  // a: <flag-list>, list of flags which affect output:
  //    - 'w' requires 'e', means that output file will have graph edges with weights (either parsed or random)
  //    - 's' requires 'e', means that output file will start with number vertexes in a graph
  //
  const char* options = "i:o:t:p:a:r:d:";

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

        opt_parse = optarg;
        break;
      case 'a':
        std::cerr << "-a: " << optarg << "\n";

        opt_add = optarg;
        break;
      case 'd':
        std::cerr << "-d: " << optarg << "\n";

        opt_delta = atoi(optarg);
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

  bool opt_parse_w = false, opt_parse_s = false;
  if (!parse(opt_parse, opt_parse_w, opt_parse_s)) {
    std::cerr << "erro: unsupported input type in '-p' option, must be 'w', 'c' or their combinations";
    return 1;
  }
  if (!validate_parse(opt_type, opt_parse_w, opt_parse_s)) {
    std::cerr << "erro: unsupported combination of '-p' and '-t'";
    return 1;
  }

  bool opt_add_w = false, opt_add_s = false;
  if (!parse(opt_add, opt_add_w, opt_add_s)) {
    std::cerr << "erro: unsupported input type in '-a' option, must be 'w', 'c' or their combinations";
    return 1;
  }
  if (!validate_parse(opt_type, opt_parse_w, opt_parse_s)) {
    std::cerr << "erro: unsupported combination of '-a' and '-t'";
    return 1;
  }

  std::function<bool(std::istream&, context&, parameters)> read;
  std::function<bool(std::ostream&, context&, parameters)> write;

  int wmask = opt_parse_w << 1 | opt_add_w;
  switch (wmask) {
    case 0: // false
      read  = read_ft;
      write = write_ft;
      break;
    case 1: // opt_add_w
      read  = read_ftr;
      write = write_ftw;
      break;
    case 2: // opt_parse_w
      read  = read_ftw;
      write = write_ft;
      break;
    case 3: // true
      read  = read_ftw;
      write = write_ftw;
      break;
  }

  // We use uniform distribution to get random weight values
  //
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

  std::mt19937_64                    weight_distribution_engine(seed);
  std::uniform_int_distribution<int> weight_distribution(opt_low_weight, opt_high_weight);

  // We open both files
  //
  std::ifstream input_stream(opt_input);
  std::ofstream output_stream(opt_output);

  // We prepare global parameters
  //
  parameters params = {
    .delta               = opt_delta,
    .distribution_engine = weight_distribution_engine,
    .distribution        = weight_distribution
  };

  int size;
  if (opt_parse_s) {
    input_stream >> size;
  }

  if (opt_add_s) {
    if (opt_parse_s) {
      output_stream << size;
    } else {
      int min = 1, max = 0;

      context ctx;
      while (read(input_stream, ctx, params)) {
        max = std::max({ max, ctx.f, ctx.t });
        min = std::min({ min, ctx.f, ctx.t });
      }
      // If we count vertex numbers from zero, then the number of vertexes
      // would be max mentioned vertex + 1
      //
      if (min == 0)
        max++;

      output_stream << max << '\n';

      input_stream.clear();
      input_stream.seekg(std::ios::beg);
    }
  }

  switch (opt_type) {
    case 'e':
      context ctx;
      while (read(input_stream, ctx, params))
        write(output_stream, ctx, params);
      break;
    case 'c':
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

  output_stream.flush();
}

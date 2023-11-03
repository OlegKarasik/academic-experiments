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

// This is a tiny program which can modify the graph or
// clusters described in a simple text format
//
int
main(int argc, char* argv[])
{
  std::string input;
  std::string output;
  std::string command;
  std::string type;

  // Supported options
  // i: <path>,   path to input graph (without weights, only edges)
  // o: <path>,   path to output graph (without weights, only edges)
  // t: <string>, type of input graph
  //    Supported values:
  //    - 'd' means directed, the input is in the following form:
  //          <from> <to>
  //          <from> <to>
  //          ...
  //    - 'c' means clusteres, the input is in the following form:
  //          <vertex> <vertex> <vertex>
  //          <vertex> <vertex>
  //          ...
  // c: <string>, command to run
  //    Supported values:
  //    - 'inc' to increment vertex indexes by 1
  //    - 'dec' to decrement vertex indexes by 1
  //
  const char* options = "i:o:c:t:";

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
      case 'c':
        std::cerr << "-c: " << optarg << "\n";

        command = optarg;
        break;
      case 't':
        std::cerr << "-t: " << optarg << "\n";

        type = optarg;
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
  if (command.empty()) {
    std::cerr << "erro: the -c parameter is required";
    return 1;
  }
  if (command != "inc" && command != "dec") {
    std::cerr << "erro: the -c parameter must be 'inc' or 'dec'";
    return 1;
  }
  if (type != "d" && type != "c") {
    std::cerr << "erro: the -t parameter must be 'd' or 'c'";
    return 1;
  }

  std::ifstream inps;
  std::ofstream outs;

  inps.open(input);
  outs.open(output);



  // Read (`from vertex` `to vertex`) and write (`from vertex` `to vertex` `weight`)
  //
  int delta;

  if (command == "inc") delta = 1;
  if (command == "dec") delta = -1;

  if (type == "d") {
    int f, t;
    while (inps >> f >> t) {
      outs << (f + delta) << ' ' << (t + delta) << '\n';
    }
  }
  if (type == "c") {
    std::string line;
    while (std::getline(inps, line)) {
      int v;

      std::istringstream iss(line);
      while (iss >> v) {
        outs << (v + delta) << ' ';
      }

      outs << '\n';
    }
  }

  outs.flush();
}

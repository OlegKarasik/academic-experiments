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

// This is a tiny program which modifies the graph by either
// incrementing or decrementing all numbers by 1
//
int
main(int argc, char* argv[])
{
  std::string input;
  std::string output;
  std::string command;

  // Supported options
  // i: <path>,   path to input graph (without weights, only edges)
  // o: <path>,   path to output graph (without weights, only edges)
  // c: <string>, command to run
  //    Supported values:
  //    - 'inc' to increment vertex indexes by 1
  //    - 'dec' to decrement vertex indexes by 1
  //
  const char* options = "i:o:c:";

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

  std::ifstream inps;
  std::ofstream outs;

  inps.open(input);
  outs.open(output);

  // Read (`from vertex` `to vertex`) and write (`from vertex` `to vertex` `weight`)
  //
  int f, t, delta;

  if (command == "inc") delta = 1;
  if (command == "dec") delta = -1;

  while (inps >> f >> t) {
    outs << (f + delta) << ' ' << (t + delta) << '\n';
  }

  outs.flush();
}

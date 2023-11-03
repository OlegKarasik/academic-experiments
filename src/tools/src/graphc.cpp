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
#include <set>
#include <map>

// global C includes
//
#include <stdlib.h>
#ifdef _INTEL_COMPILER
  #include <io.h>

  #include "portables/posix/getopt.h"
#else
  #include <unistd.h>
#endif

// This is a tiny program which identifies the number of directed
// endges between clusters using the clusters + directed graph input
//
int
main(int argc, char* argv[])
{
  std::string input_g;
  std::string input_c;
  std::string output;

  // Supported options
  // g: <path>, path to input graph (without weights, only edges)
  // c: <path>, path to input clusters (without weights, only edges)
  // o: <path>, path to output
  //
  const char* options = "g:c:o:";

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'g':
        std::cerr << "-g: " << optarg << "\n";

        input_g = optarg;
        break;
      case 'c':
        std::cerr << "-c: " << optarg << "\n";

        input_c = optarg;
        break;
      case 'o':
        std::cerr << "-o: " << optarg << "\n";

        output = optarg;
        break;
    }
  }

  if (input_g.empty()) {
    std::cerr << "erro: the -g parameter is required";
    return 1;
  }
  if (input_c.empty()) {
    std::cerr << "erro: the -c parameter is required";
    return 1;
  }
  if (output.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }

  std::ifstream inps_g;
  std::ifstream inps_c;
  std::ofstream outs;

  inps_g.open(input_g);
  inps_c.open(input_c);
  outs.open(output);

  std::vector<std::set<int>> clusters;
  std::map<std::pair<int, int>, std::vector<int>> intersections;

  // Read all clusters
  //
  {
    std::string line;
    while (std::getline(inps_c, line)) {
      int v;

      std::set<int> items;

      std::istringstream iss(line);
      while (iss >> v) {
        items.insert(v);
      }

      clusters.emplace_back(items);
    }
  }
  // Read edges one by one and map them to clusters
  //
  {
    int id = 0;

    std::string line;
    while (std::getline(inps_g, line)) {
      int f, t;

      std::istringstream iss(line);

      iss >> f >> t;

      bool found = false;
      for (auto it_f = clusters.begin(); it_f != clusters.end() && !found; ++it_f) {
        auto _f = it_f->find(f);
        if (_f != it_f->end()) {
          for (auto it_t = clusters.begin(); it_t != clusters.end() && !found; ++it_t) {
            if (it_f == it_t)
              continue;

            auto _t = it_t->find(t);
            if (_t != it_t->end()) {
              std::pair<int, int> key(std::distance(clusters.begin(), it_f), std::distance(clusters.begin(), it_t));

              auto _m = intersections.find(key);
              if (_m == intersections.end()) {
                std::vector<int> value { id };

                intersections.emplace(key, value);
              } else {
                _m->second.push_back(id);
              }
              found = true;
            }
          }
        }
      }

      ++id;
    }
  }
  // Save
  //
  for (auto kv : intersections) {
    outs << kv.first.first   << ' ' << kv.first.second << " "
         << kv.second.size() << " " << clusters[kv.first.first].size();

    outs << '\n';
  }

  outs.flush();
}

// portability
#include "portables/hacks/defines.h"

// global includes
//
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
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

// This is a tiny program which performs an analysis of graphs
//
int
main(int argc, char* argv[])
{
  std::string opt_input_edges;
  std::string opt_input_clusters;
  std::string opt_output;

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

        opt_output = optarg;
        break;
    }
  }

  if (opt_input_edges.empty()) {
    std::cerr << "erro: the -i parameter is required";
    return 1;
  }
  if (opt_output.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }

  std::ifstream edges_stream(opt_input_edges);
  std::ofstream output_stream(opt_output);

  std::vector<std::set<int>>                      clusters;
  std::map<std::pair<int, int>, std::vector<int>> intersections;

  if (!opt_input_clusters.empty()) {
    // Read all clusters
    //
    {
      std::ifstream clusters_stream(opt_input_clusters);

      std::string line;
      while (std::getline(clusters_stream, line)) {
        int v;

        std::set<int> items;

        std::istringstream iss(line);
        while (iss >> v) {
          items.insert(v);
        }

        clusters.emplace_back(items);
      }
    }
    // Recalculate correlation of edges between clusters
    //
    {
      int id = 0;

      std::string line;
      while (std::getline(edges_stream, line)) {
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
                  std::vector<int> value{ id };

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

      output_stream << "Count of clusters with count of vertex within\n"
                    << "Read as following: <vertex count> <cluster count>\n";

      std::map<int, int> cluster_counts;
      for (auto cluster : clusters) {
        auto _m = cluster_counts.find(cluster.size());
        if (_m == cluster_counts.end()) {
          cluster_counts.emplace(cluster.size(), 1);
        } else {
          ++_m->second;
        }
      }
      for (auto kv : cluster_counts) {
        output_stream << kv.first << ' ' << kv.second << " ";

        output_stream << '\n';
      }

      output_stream << '\n';

      output_stream << "Count of edges between clusters\n"
                    << "Read as following: <source> <destination> <edges in between> <edges in source>\n";

      for (auto kv : intersections) {
        output_stream << kv.first.first << ' ' << kv.first.second << " "
                      << kv.second.size() << " " << clusters[kv.first.first].size();

        output_stream << '\n';
      }
    }
  }

  output_stream.flush();
}

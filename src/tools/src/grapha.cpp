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

#include "communities-io.hpp"
#include "graphs-io.hpp"
#include "square-shape.hpp"

using Index = long;
using Value = long;

enum analysis_options
{
  analysis_opt_communities_intersections
};

void
analyse_communities_intersections(
  std::vector<std::tuple<Index, Index>> graph_vector,
  std::vector<std::set<Index>>          communities_vector);

// This is a tiny program which performs an analysis of graphs
//
int
main(int argc, char* argv[])
{
  utilz::graphs::io::graph_format            opt_graph_format       = utilz::graphs::io::graph_format::graph_fmt_none;
  utilz::communities::io::communities_format opt_communities_format = utilz::communities::io::communities_format::communities_fmt_none;

  std::string opt_input_graph;
  std::string opt_input_communities;
  std::string opt_output;

  // Supported options
  // g: <path>, path to a graph file
  // G: <enum>, format of a graph file
  //    Supported values:
  //    - 'edgelist'
  //    - 'dimacs'
  //    - 'weightlist'
  //    - 'binary'
  // c: <path>, path to a communities file
  // C: <enum>, format of a communities file
  //    Supported values:
  //    - 'rlang'

  // o: <path>, path to output file
  //
  const char* options = "g:G:c:C:o:";

  std::cerr << "Options:\n";

  int opt;
  while ((opt = getopt(argc, argv, options)) != -1) {
    switch (opt) {
      case 'g':
        if (opt_input_graph.empty()) {
          std::cerr << "-g: " << optarg << "\n";

          opt_input_graph = optarg;
          break;
        }
        std::cerr << "erro: unexpected '-g' option detected" << '\n';
        return 1;
      case 'c':
        if (opt_input_communities.empty()) {
          std::cerr << "-c: " << optarg << "\n";

          opt_input_communities = optarg;
          break;
        }
        std::cerr << "erro: unexpected '-c' option detected" << '\n';
        return 1;
      case 'G':
        if (opt_graph_format == utilz::graphs::io::graph_format::graph_fmt_none) {
          std::cerr << "-G: " << optarg << "\n";

          if (!utilz::graphs::io::parse_graph_format(optarg, opt_graph_format)) {
            std::cerr << "erro: invalid graph format has been detected in '-G' option" << '\n';
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-G' option detected" << '\n';
        return 1;
      case 'C':
        if (opt_communities_format == utilz::communities::io::communities_format::communities_fmt_none) {
          std::cerr << "-C: " << optarg << "\n";

          if (!utilz::communities::io::parse_communities_format(optarg, opt_communities_format)) {
            std::cerr << "erro: invalid communities format has been detected in '-C' option" << '\n';
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-C' option detected" << '\n';
        return 1;
      case 'o':
        std::cerr << "-o: " << optarg << "\n";

        if (opt_output.empty()) {
          opt_output = optarg;
          break;
        }
        std::cerr << "erro: unexpected '-o' option detected" << '\n';
        return 1;
    }
  }

  if (!opt_input_graph.empty() && opt_graph_format == utilz::graphs::io::graph_fmt_none ||
      opt_input_graph.empty() && opt_graph_format != utilz::graphs::io::graph_fmt_none) {
    std::cerr << "erro: the -g and -G parameters must be both set";
    return 1;
  }
  if (!opt_input_communities.empty() && opt_communities_format == utilz::communities::io::communities_fmt_none ||
      opt_input_communities.empty() && opt_communities_format != utilz::communities::io::communities_fmt_none) {
    std::cerr << "erro: the -g and -G parameters must be both set";
    return 1;
  }
  if (opt_output.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }

  std::vector<std::tuple<Index, Index>> graph_vector;
  std::map<Index, std::set<Index>>      communities_vector;

  if (!opt_input_graph.empty()) {
    auto set_w = std::function([](std::vector<std::tuple<Index, Index>>& c, Index f, Index t, Value w) -> void {
      c.emplace_back(std::make_tuple(f, t));
    });

    std::ifstream graph_stream(opt_input_graph);

    utilz::graphs::io::scan_graph(opt_graph_format, graph_stream, graph_vector, set_w);
  }
  if (!opt_input_communities.empty()) {
    auto set_v = std::function([](std::map<Index, std::set<Index>>& c, Index ci, Index vi) -> void {
      auto set = c.find(ci);
      if (set == c.end()) {
        c.emplace(ci, std::set<Index>({ vi }));
      } else {
        set->second.insert(vi);
      }
    });

    std::ifstream communities_stream(opt_input_communities);

    utilz::communities::io::scan_communities(opt_communities_format, communities_stream, communities_vector, set_v);
  }

  std::ifstream graph_stream(opt_input_graph);
  std::ofstream output_stream(opt_output);

  utilz::graphs::io::graph_edge<utilz::graphs::io::graph_format::graph_fmt_edgelist, int, int> edzz;
  graph_stream >> edzz;

  std::vector<std::set<int>>                      clusters;
  std::map<std::pair<int, int>, std::vector<int>> intersections;

  if (!opt_input_communities.empty()) {
    // Read all clusters
    //
    {
      auto set_vertex = [](std::vector<long> v, long f, long t, long w) -> void {
      };

      std::vector<long> vec_p;

      std::ifstream clusters_stream(opt_input_communities);

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
      while (std::getline(graph_stream, line)) {
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

void
analyse_communities_intersections(
  std::vector<std::tuple<Index, Index>> graph_vector,
  std::vector<std::set<Index>>          communities_vector)
{
  utilz::square_shape<Index> adjacency_matrix;
};

// portability
#include "portables/hacks/defines.h"

// global includes
//
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
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
  std::ostream&                     os,
  utilz::square_shape<Index>&       graph_matrix,
  std::map<Index, std::set<Index>>& communities_map);

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

  utilz::square_shape<Index>       graph_matrix;
  std::map<Index, std::set<Index>> communities_map;

  if (!opt_input_graph.empty()) {
    auto set_vc = std::function([](utilz::square_shape<Index>& c, Index vc) -> void {
      utilz::procedures::square_shape_set_size<utilz::square_shape<Index>> set_size;
      set_size(c, vc);
    });
    auto set_ec = std::function([](utilz::square_shape<Index>& c, Index ec) -> void {
    });
    auto set_w  = std::function([](utilz::square_shape<Index>& c, Index f, Index t, Value w) -> void {
      utilz::procedures::square_shape_at<utilz::square_shape<Index>> at;
      at(c, f, t) = w;
    });

    std::ifstream graph_stream(opt_input_graph);
    if (!graph_stream.is_open()) {
      std::cerr << "erro: can't open graph file (denoted by -g option)";
      return 1;
    }

    utilz::graphs::io::scan_graph(opt_graph_format, graph_stream, graph_matrix, set_vc, set_ec, set_w);
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
    if (!communities_stream.is_open()) {
      std::cerr << "erro: can't open communities file (denoted by -G option)";
      return 1;
    }

    utilz::communities::io::scan_communities(opt_communities_format, communities_stream, communities_map, set_v);
  }

  std::ofstream output_stream(opt_output);

  analyse_communities_intersections(output_stream, graph_matrix, communities_map);

  output_stream.flush();
}

void
analyse_communities_intersections(
  std::ostream&                     os,
  utilz::square_shape<Index>&       graph_matrix,
  std::map<Index, std::set<Index>>& communities_map)
{
  auto edge_count = size_t(0);
  for (auto i = 0; i < graph_matrix.size(); ++i)
    for (auto j = 0; j < graph_matrix.size(); ++j)
      if (graph_matrix.at(i, j) != 0)
        edge_count++;

  std::map<Index, double> communities_pvt;
  std::map<Index, double> communities_pet;
  std::map<Index, size_t> communities_bv;
  std::map<Index, size_t> communities_ev;
  std::map<Index, size_t> communities_ec;

  std::map<Index, std::map<Index, size_t>> communities_bvc;
  std::map<Index, std::map<Index, size_t>> communities_bec;

  for (auto community : communities_map) {
    communities_pvt.emplace(community.first, double(0));
    communities_pet.emplace(community.first, double(0));
    communities_bv.emplace(community.first, size_t(0));
    communities_ev.emplace(community.first, size_t(0));
    communities_ec.emplace(community.first, size_t(0));
  }

  for (auto community : communities_map) {
    auto pvt = communities_pvt.find(community.first);
    auto pet = communities_pet.find(community.first);

    auto bv = communities_bv.find(community.first);
    auto ev = communities_ev.find(community.first);
    auto ec = communities_ec.find(community.first);

    for (auto i : community.second) {
      auto edges = graph_matrix.at(i);

      auto f = false;
      for (auto j = utilz::square_shape<Index>::size_type(0); j < graph_matrix.size(); ++j) {
        if (edges[j] != Index(0)) {
          ec->second++;
          for (auto c : communities_map) {
            if (community.first != c.first && c.second.find(j) != c.second.end()) {
              if (!f) {
                bv->second++;
                f = true;
              }
              ev->second++;
              break;
            }
          }
        }
      }
    }

    pvt->second = static_cast<double>(community.second.size()) / graph_matrix.size() * 100;
    pet->second = static_cast<double>(ec->second) / edge_count * 100;
  }

  os << "== COMMUNITIES INTERSECTION ANALYSIS ==\n";
  os << "VT:" << std::setw(8) << graph_matrix.size() << "\n";
  os << "ET:" << std::setw(8) << edge_count << "\n\n";
  os << std::setw(6) << "Index"
     << " "
     << std::setw(8) << "V"
     << " "
     << std::setw(8) << "% (VT)"
     << " "
     << std::setw(8) << "E"
     << " "
     << std::setw(8) << "% (ET)"
     << " "
     << std::setw(8) << "BV"
     << " "
     << std::setw(8) << "BE"
     << "\n";

  for (auto community : communities_map) {
    os << "[" << std::setw(4) << community.first << "]"
       << " "
       << std::setw(8) << community.second.size()
       << " "
       << std::setw(8) << std::setprecision(2) << std::fixed << communities_pvt[community.first]
       << " "
       << std::setw(8) << communities_ec[community.first]
       << " "
       << std::setw(8) << std::setprecision(2) << std::fixed << communities_pet[community.first]
       << " "
       << std::setw(8) << communities_bv[community.first]
       << " "
       << std::setw(8) << communities_ev[community.first]
       << "\n";
  }
};

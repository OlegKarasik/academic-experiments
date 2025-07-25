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

#include "constants.hpp"

#include "communities-io.hpp"
#include "graphs-io.hpp"

#include "matrix-io.hpp"
#include "matrix.hpp"

namespace utzmx  = ::utilz::matrices;
namespace utzgio = ::utilz::graphs::io;
namespace utzcio = ::utilz::communities::io;

using Index = long;
using Value = long;

enum analysis_options
{
  analysis_opt_communities_intersections
};

void
analyse_communities_intersections(
  std::ostream&                     os,
  utzmx::square_matrix<Index>&      graph_matrix,
  std::map<Index, std::vector<Index>>& communities_map);

// This is a tiny program which performs an analysis of graphs
//
int
main(int argc, char* argv[])
{
  utzgio::graph_format       opt_graph_format       = utzgio::graph_format::graph_fmt_none;
  utzcio::communities_format opt_communities_format = utzcio::communities_format::communities_fmt_none;

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
        if (opt_graph_format == utzgio::graph_format::graph_fmt_none) {
          std::cerr << "-G: " << optarg << "\n";

          if (!utzgio::parse_graph_format(optarg, opt_graph_format)) {
            std::cerr << "erro: invalid graph format has been detected in '-G' option" << '\n';
            return 1;
          }
          break;
        }
        std::cerr << "erro: unexpected '-G' option detected" << '\n';
        return 1;
      case 'C':
        if (opt_communities_format == utzcio::communities_format::communities_fmt_none) {
          std::cerr << "-C: " << optarg << "\n";

          if (!utzcio::parse_communities_format(optarg, opt_communities_format)) {
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

  if (!opt_input_graph.empty() && opt_graph_format == utzgio::graph_fmt_none ||
       opt_input_graph.empty() && opt_graph_format != utzgio::graph_fmt_none) {
    std::cerr << "erro: the -g and -G parameters must be both set";
    return 1;
  }
  if (!opt_input_communities.empty() && opt_communities_format == utzcio::communities_fmt_none ||
       opt_input_communities.empty() && opt_communities_format != utzcio::communities_fmt_none) {
    std::cerr << "erro: the -c and -C parameters must be both set";
    return 1;
  }
  if (opt_output.empty()) {
    std::cerr << "erro: the -o parameter is required";
    return 1;
  }

  utzmx::square_matrix<Index> graph_matrix;
  utzmx::matrix_abstract<utzmx::square_matrix<Index>> graph_matrix_abstract(graph_matrix);

  std::map<Index, std::vector<Index>> communities_map;

  if (!opt_input_graph.empty()) {
    std::ifstream graph_stream(opt_input_graph);
    if (!graph_stream.is_open()) {
      std::cerr << "erro: can't open graph file (denoted by -g option)";
      return 1;
    }

    utzmx::io::scan_matrix(
      opt_graph_format,
      graph_stream,
      graph_matrix_abstract);
  }
  if (!opt_input_communities.empty()) {
    std::ifstream communities_stream(opt_input_communities);
    if (!communities_stream.is_open()) {
      std::cerr << "erro: can't open communities file (denoted by -c option)";
      return 1;
    }

    communities_map = utzcio::scan_communities<Index>(opt_communities_format, communities_stream);
  }

  std::ofstream output_stream(opt_output);

  analyse_communities_intersections(output_stream, graph_matrix, communities_map);

  // Flush output stream
  //
  output_stream.flush();
}

void
analyse_communities_intersections(
  std::ostream&                        os,
  utzmx::square_matrix<Index>&         graph_matrix,
  std::map<Index, std::vector<Index>>& communities_map)
{
  auto total_bv  = size_t(0);
  auto total_be  = size_t(0);
  auto total_pbv = double(0);
  auto total_pbe = double(0);

  auto vertex_count = graph_matrix.size();
  auto edge_count   = size_t(0);
  for (auto i = 0; i < graph_matrix.size(); ++i)
    for (auto j = 0; j < graph_matrix.size(); ++j)
      if (graph_matrix.at(i, j) != utilz::constants::infinity<Index>())
        edge_count++;

  std::map<Index, std::set<Index>>                      communities_path;
  std::map<Index, std::vector<std::pair<Index, Index>>> communities_ranges;
  std::map<Index, std::vector<std::pair<Index, Index>>> communities_connections;

  std::map<Index, double> communities_pvt;
  std::map<Index, double> communities_pet;

  std::map<Index, std::set<Index>> communities_bv;
  std::map<Index, std::set<Index>> communities_bvi;
  std::map<Index, std::set<Index>> communities_bvo;
  std::map<Index, size_t>          communities_be;
  std::map<Index, size_t>          communities_ec;

  for (auto community : communities_map) {
    communities_path.emplace(community.first, std::set<Index>());
    communities_ranges.emplace(community.first, std::vector<std::pair<Index, Index>>());
    communities_connections.emplace(community.first, std::vector<std::pair<Index, Index>>());

    communities_pvt.emplace(community.first, double(0));
    communities_pet.emplace(community.first, double(0));

    communities_bv.emplace(community.first, std::set<Index>());
    communities_bvi.emplace(community.first, std::set<Index>());
    communities_bvo.emplace(community.first, std::set<Index>());
    communities_be.emplace(community.first, size_t(0));
    communities_ec.emplace(community.first, size_t(0));
  }

  for (auto community : communities_map) {
    auto paths       = communities_path.find(community.first);
    auto connections = communities_connections.find(community.first);

    auto pvt = communities_pvt.find(community.first);
    auto pet = communities_pet.find(community.first);

    auto bv  = communities_bv.find(community.first);
    auto bvi = communities_bvi.find(community.first);
    auto bvo = communities_bvo.find(community.first);
    auto be  = communities_be.find(community.first);
    auto ec  = communities_ec.find(community.first);

    for (auto i : community.second) {
      auto edges = graph_matrix.at(i);

      for (auto j = utzmx::square_matrix<Index>::size_type(0); j < graph_matrix.size(); ++j) {
        // If there is an edge between vertices (because we load adjucency matrix we treat all infities as none)
        //
        if (edges[j] != utilz::constants::infinity<Index>()) {
          // Increment community edge count (both inner and outer)
          //
          ec->second++;

          // If edge is within community, then we skip communities check and continue
          //
          if (std::find(community.second.begin(), community.second.end(), j) != community.second.end())
            continue;

          auto sanity_edges = false;
          for (auto c : communities_map) {
            if (community.first != c.first && std::find(c.second.begin(), c.second.end(), j) != c.second.end()) {
              auto reverse_bv  = communities_bv.find(c.first);
              auto reverse_bvi = communities_bvi.find(c.first);
              // Increment bridge edges count
              //
              be->second++;

              // Save the connection between two communities (from `community` to `c`)
              //
              paths->second.insert(c.first);

              // Save the connecting edge (from `community` to `c`)
              //
              connections->second.push_back(std::make_pair(i, j));

              // Save the output bridge vertex (for `community`)
              //
              bv->second.insert(i);
              bvo->second.insert(i);

              // Save the input bridge vertex (for `c`)
              //
              reverse_bv->second.insert(j);
              reverse_bvi->second.insert(j);

              sanity_edges = true;
              break;
            }
          }
          if (!sanity_edges)
            throw std::logic_error("erro: the graph contains edges which doesn't match community structure [" + std::to_string(i) + ", " + std::to_string(j) + "]");
        }
      }
    }

    pvt->second = static_cast<double>(community.second.size()) / vertex_count * 100;
    pet->second = static_cast<double>(ec->second) / edge_count * 100;
  }

  for (auto community : communities_map) {
    auto ranges = communities_ranges.find(community.first);

    auto first = false;
    auto start = Index(0), end = Index(0);
    for (auto v : community.second) {
      if (!first) {
        first = true;
        start = end = v;
      } else {
        if (v == (end + 1)) {
          end = v;
        } else {
          ranges->second.emplace_back(std::make_pair(start, end));

          start = end = v;
        }
      }
    }
    if (first)
      ranges->second.emplace_back(std::make_pair(start, end));
  }

  total_bv = std::accumulate(
    communities_bv.begin(),
    communities_bv.end(),
    size_t(0),
    [](auto acc, auto it) -> size_t {
      return acc + it.second.size();
    });

  total_be = std::accumulate(
    communities_be.begin(),
    communities_be.end(),
    size_t(0),
    [](auto acc, auto it) -> size_t {
      return acc + it.second;
    });

  total_pbv = static_cast<double>(total_bv) / vertex_count * 100;
  total_pbe = static_cast<double>(total_be) / edge_count * 100;

  os << "== COMMUNITIES INTERSECTION ANALYSIS ==\n"
     << "VT        :" << std::setw(8) << vertex_count << "\n"
     << "ET        :" << std::setw(8) << edge_count << "\n"
     << "BV        :" << std::setw(8) << total_bv << "\n"
     << "BV % (VT) :" << std::setw(8) << std::setprecision(2) << total_pbv << "\n"
     << "BE        :" << std::setw(8) << total_be << "\n"
     << "BE % (ET) :" << std::setw(8) << std::setprecision(2) << total_pbe << "\n"
     << "\n";

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
     << " "
     << std::setw(8) << "C"
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
       << std::setw(8) << communities_bv[community.first].size()
       << " "
       << std::setw(8) << communities_be[community.first]
       << " "
       << std::setw(8) << communities_path[community.first].size()
       << "\n";
  }

  os << "\n"
     << "Connections between clusters (connections)\n"
     << "\n";

  for (auto community : communities_map) {
    auto paths = communities_path.find(community.first);

    os << "[" << std::setw(4) << paths->first << "]: ";

    if (paths->second.empty())
      os << "No outbound paths";
    else {
      for (auto c : paths->second)
        os << c << " ";
    }

    os << "\n";
  }

  os << "\n"
     << "Connections between clusters (edges)\n"
     << "\n";

  for (auto community : communities_map) {
    auto connections = communities_connections.find(community.first);

    os << "[" << std::setw(4) << connections->first << "]: ";

    if (connections->second.empty())
      os << "No outbound connections";
    else {
      for (auto c : connections->second)
        os << "[" << c.first << "->" << c.second << "] ";
    }

    os << "\n";
  }

  os << "\n"
     << "Clusters vertices\n"
     << "\n";

  for (auto community : communities_map) {
    auto ranges = communities_ranges.find(community.first);

    os << "[" << std::setw(4) << ranges->first << "]: ";

    for (auto range : ranges->second) {
      if (range.first == range.second) {
        // Print vertex
        //
        os << range.first << " ";
      } else {
        // Print range
        //
        os << range.first << "-" << range.second << " ";
      }
    }

    os << "\n";
  }

  os << "\n"
     << "Clusters bridge vertices\n"
     << "\n";

  for (auto community : communities_map) {
    auto bv  = communities_bv.find(community.first);
    auto bvi = communities_bvi.find(community.first);
    auto bvo = communities_bvo.find(community.first);

    os << "[" << std::setw(4) << community.first << "]" << "\n";

    os << "  All    : ";
    if (bv->second.empty())
      os << "No bridge vertices" << "\n";
    else {
      for (auto c : bv->second)
        os << c << " ";
    }
    os << "\n";

    os << "  Input  : ";
    if (bvi->second.empty())
      os << "No input bridge vertices" << "\n";
    else {
      for (auto c : bvi->second)
        os << c << " ";
    }
    os << "\n";

    os << "  Output : ";
    if (bvo->second.empty())
      os << "No output bridge vertices" << "\n";
    else {
      for (auto c : bvo->second)
        os << c << " ";
    }

    os << "\n";
  }
};

#pragma once

#include "graphs-io.hpp"

#include <exception>
#include <filesystem>
#include <fstream>

namespace utilz {
namespace graphs {
namespace io {

template<typename GraphT, typename WeightT, typename ResizeOp, typename EdgeOp>
void
fscan_graph(const std::filesystem::path& path, GraphT& g, ResizeOp& resize, EdgeOp& edge)
{
  std::ifstream s(path, std::ifstream::in);
  if (!s.is_open())
    throw std::logic_error("erro: can't open '" + path.string() + "' text file in read mode");

  scan_graph<GraphT, WeightT, ResizeOp, EdgeOp>(s, g, resize, edge);
}

template<typename GraphT, typename WeighT, typename CountOp, typename EdgeOp>
void
fprint_graph(const std::filesystem::path& path, GraphT& g, CountOp& count, EdgeOp& edge)
{
  std::ofstream s(path, std::ofstream::out);
  if (!s.is_open())
    throw std::logic_error("erro: can't open '" + path.string() + "' text file in read mode");

  print_graph<GraphT, WeighT, CountOp, EdgeOp>(s, g, count, edge);
};

} // namespace io
} // namespace graphs
} // namespace utilz

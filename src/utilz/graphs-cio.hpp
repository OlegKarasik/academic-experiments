#pragma once

#include "graphs-io.hpp"

#include <iostream>

namespace utilz {
namespace graphs {
namespace io {

template<typename GraphT, typename WeightT, typename ResizeOp, typename EdgeOp>
void
cscan_graph(GraphT& g, ResizeOp& resize, EdgeOp& edge)
{
  scan_graph<GraphT, WeightT, ResizeOp, EdgeOp>(std::cin, g, resize, edge);
}

template<typename GraphT, typename WeighT, typename CountOp, typename EdgeOp>
void
cprint_graph(GraphT& g, CountOp& count, EdgeOp& edge)
{
  print_graph<GraphT, WeighT, CountOp, EdgeOp>(std::cout, g, count, edge);
};

} // namespace io
} // namespace graphs
} // namespace utilz

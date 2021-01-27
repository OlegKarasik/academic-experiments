#pragma once

#include "graphs-io.hpp"

#include <iostream>

namespace utilz {
namespace graphs {
namespace io {

template<typename T, typename O>
void
cscan_graph(O& out)
{
  scan_graph<T, O>(std::cin, out);
}

template<typename T, typename I>
void
cprint_graph(I& in)
{
  print_graph<T, I>(std::cout, in);
};

} // namespace io
} // namespace graphs
} // namespace utilz

#pragma once

#include "graphs-io.hpp"

#include <fstream>

namespace utilz {
namespace graphs {
namespace io {

template<typename T, typename O>
void
fscan_graph(const std::string& path, O& out)
{
  std::ifstream s(path, std::ifstream::in);
  if (!s.is_open())
    throw std::runtime_error("erro: can't open '" + path + "' text file in read mode");

  scan_graph<T, O>(s, out);
}

template<typename T, typename I>
void
fprint_graph(const std::string& path, I& in)
{
  std::ofstream s(path, std::ofstream::out);
  if (!s.is_open())
    throw std::runtime_error("erro: can't open '" + path + "' text file in read mode");

  print_graph<T, I>(s, in);
};

} // namespace io
} // namespace graphs
} // namespace utilz

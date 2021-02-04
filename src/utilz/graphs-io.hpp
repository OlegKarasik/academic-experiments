#pragma once

#include <iostream>

namespace utilz {
namespace graphs {
namespace io {

template<typename GraphT, typename WeightT, typename ResizeOp, typename EdgeOp>
void
scan_graph(std::istream& s, GraphT& g, ResizeOp& resize, EdgeOp& edge)
{
  // Read vertex and edge count
  //
  size_t v = 0, e = 0;
  if (!(s >> v >> e)) {
    throw std::logic_error("erro: the data is in invalid format, "
                           "the expected format is <vertex> <edge>");
  }

  // Resize graph
  //
  resize(g, v, e);

  // While we can, keep reading edges (`from vertex` `to vertex` `the weight`)
  //
  WeightT w;
  size_t  f, t;
  while (s >> f >> t >> w)
    edge(g, f, t) = w;

  if (!s.eof()) {
    throw std::logic_error("erro: the data is in invalid format, "
                           "the expected format is <from> <to> <weight>");
  }
};

template<typename GraphT, typename WeightT, typename CountOp, typename EdgeOp>
void
print_graph(std::ostream& s, GraphT& g, CountOp& count, EdgeOp& edge)
{
  // Retrieve vertex and edges counts
  //
  size_t v, e;
  count(g, v, e);

  // Write vertex and edge count information
  //
  if (!(s << v << ' ' << e << std::endl))
    throw std::logic_error("erro: can't write <vertex> and <edge> count");

  // Iterate over graph edges and write edge information
  // in form of `from vertex` `to vertex` `the weight`
  //

  WeightT w;
  bool    r = false;
  size_t  f = 0, t = 0;

  r = edge(g, f, t, w, r);
  while (r) {
    if (!(s << f << ' ' << t << ' ' << w << std::endl))
      throw std::logic_error("erro: can't write <from> <to> <weight>");

    r = edge(g, f, t, w, r);
  }
};

} // namespace io
} // namespace graphs
} // namespace utilz

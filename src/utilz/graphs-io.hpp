#pragma once

#include <iostream>

namespace utilz {
namespace graphs {
namespace io {

template<typename T, typename O>
void
scan_graph(std::istream& s, O& out)
{
  // Read vertex and edge count
  //
  size_t v = 0, e = 0;
  if (!(s >> v >> e)) {
    throw std::logic_error("erro: the data is in invalid format, "
                           "the expected format is <vertex> <edge>");
  }

  // Initialize output
  //
  out.init(v, e);

  // While we can, keep reading edges (`from vertex` `to vertex` `the weight`)
  //
  T      w;
  size_t f, t;
  while (s >> f >> t >> w)
    out.set(f, t) = w;

  if (!s.eof()) {
    throw std::logic_error("erro: the data is in invalid format, "
                           "the expected format is <from> <to> <weight>");
  }
};

template<typename T, typename I>
void
print_graph(std::ostream& s, I& in)
{
  // Write vertex and edge count information
  //
  if (!(s << in.v() << ' ' << in.e() << std::endl))
    throw std::logic_error("erro: can't write <vertex> and <edge> count");

  // Iterate over graph edges and write edge information
  // in form of `from vertex` `to vertex` `the weight`
  //
  for (auto it = in.begin(); it != in.end(); ++it) {
    auto v = *it;
    if (!(s << v.f() << ' ' << v.t() << ' ' << v.w() << std::endl))
      throw std::logic_error("erro: can't write <from> <to> <weight>");
  }
}

} // namespace io
} // namespace graphs
} // namespace utilz

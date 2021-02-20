#pragma once

#include <iostream>
#include <stdexcept>

namespace utilz {
namespace graphs {
namespace io {

template<typename Matrix, typename MatrixSetSizeOperation, typename MatrixSetValueOperation>
void
scan_matrix(std::istream& s, Matrix& g, MatrixSetSizeOperation& set_size, MatrixSetValueOperation& set_value)
{
  using size_type  = typename MatrixSetSizeOperation::result_type;
  using value_type = typename MatrixSetValueOperation::result_type;

  // Read vertex and edge count
  //
  size_type sz = size_type(0);
  if (!(s >> sz))
    throw std::logic_error("erro: can't read adjacency matrix size, expected format: <size>");

  // Resize graph
  //
  set_size(g, sz);

  // While we can, keep reading edges (`from vertex` `to vertex` `the weight`)
  //
  value_type v;
  size_type  f, t;
  while (s >> f >> t >> v)
    if (v != value_type())
      set_value(g, f, t, v);

  if (!s.eof()) {
    throw std::logic_error("erro: can't read adjacency matrix cell value, expected format: <from> <to> <weight>");
  }
};

template<typename Matrix, typename MatrixGetSizeOperation, typename MatrixGetValueOperation>
void
print_matrix(std::ostream& s, Matrix& m, MatrixGetSizeOperation& get_size, MatrixGetValueOperation& get_value)
{
  using size_type  = typename MatrixGetSizeOperation::result_type;
  using value_type = typename MatrixGetValueOperation::result_type;

  // Obtain size of the adjacency matrix
  //
  size_type sz = get_size(m);
  if (sz == size_type(0))
    return;

  // Write size information
  //
  if (!(s << sz << '\n'))
    throw std::logic_error("erro: can't write adjacency matrix size");

  for (size_type i = size_type(0); i < sz; ++i)
    for (size_type j = size_type(0); j < sz; ++j) {
      value_type v = get_value(m, i, j);
      if (v != value_type())
        if (!(s << i << ' ' << j << ' ' << v << '\n'))
          throw std::logic_error("erro: can't write adjacency matrix cell value");
    }

  s.flush();
}

} // namespace io
} // namespace graphs
} // namespace utilz

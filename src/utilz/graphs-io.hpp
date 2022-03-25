#pragma once

#include <istream>
#include <ostream>
#include <stdexcept>

namespace utilz {
namespace graphs {
namespace io {

template<typename Matrix, typename MatrixSetSizeOperation, typename MatrixSetValueOperation>
void
scan_matrix(std::istream& s, bool binary, Matrix& g, MatrixSetSizeOperation& set_size, MatrixSetValueOperation& set_value)
{
  using size_type  = typename MatrixSetSizeOperation::result_type;
  using value_type = typename MatrixSetValueOperation::result_type;

  // Read vertex and edge count
  //
  size_type sz = size_type(0);
  if (binary) {
    if (!s.read(reinterpret_cast<char*>(&sz), sizeof(size_type)))
      throw std::logic_error("erro: can't scan adjacency matrix size from binary file");
  } else {
    if (!(s >> sz))
      throw std::logic_error("erro: can't scan adjacency matrix size; expected format: <size>");
  }

  // Resize graph
  //
  set_size(g, sz);

  if (binary) {
    // Read exact number of items from stream
    //
    for (size_type i = size_type(0); i < sz; ++i)
      for (size_type j = size_type(0); j < sz; ++j) {
        value_type v;
        if (!s.read(reinterpret_cast<char*>(&v), sizeof(value_type)))
          throw std::logic_error("erro: can't scan matrix from binary file - the stream ended unexpectadly.");

        set_value(g, i, j, v);
      }
  } else {
    // Keep reading edges (`from vertex` `to vertex` `the weight`)
    // as many as possible
    //
    value_type v;
    size_type  f, t;
    while (s >> f >> t >> v)
      if (v != value_type())
        set_value(g, f, t, v);
  }
};

template<typename Matrix, typename MatrixGetSizeOperation, typename MatrixGetValueOperation>
void
print_matrix(std::ostream& s, bool binary, Matrix& m, MatrixGetSizeOperation& get_size, MatrixGetValueOperation& get_value)
{
  using size_type  = typename MatrixGetSizeOperation::result_type;
  using value_type = typename MatrixGetValueOperation::result_type;

  // Obtain size of the adjacency matrix
  //
  size_type sz = get_size(m);
  if (sz == size_type(0))
    return;

  if (binary) {
    if (!s.write(reinterpret_cast<char*>(&sz), sizeof(size_type)))
      throw std::logic_error("erro: can't print adjacency matrix size");

    for (size_type i = size_type(0); i < sz; ++i)
      for (size_type j = size_type(0); j < sz; ++j) {
        value_type v = get_value(m, i, j);
        if (!s.write(reinterpret_cast<char*>(&v), sizeof(value_type)))
          throw std::logic_error("erro: can't print adjacency matrix cell value");
      }
  } else {
    if (!(s << sz << '\n'))
      throw std::logic_error("erro: can't print adjacency matrix size");

    for (size_type i = size_type(0); i < sz; ++i)
      for (size_type j = size_type(0); j < sz; ++j) {
        value_type v = get_value(m, i, j);
        if (v != value_type())
          if (!(s << i << ' ' << j << ' ' << v << '\n'))
            throw std::logic_error("erro: can't print adjacency matrix cell value");
      }
  }

  s.flush();
};

} // namespace io
} // namespace graphs
} // namespace utilz

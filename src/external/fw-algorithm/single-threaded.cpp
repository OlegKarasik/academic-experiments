#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>
#include <string>

#include "../../utilz/io.h"
#include "../../utilz/square_shape.h"

using namespace utilz;

void
_impl(square_shape<long>& shape)
{
  for (size_t k = 0; k < shape.s(); ++k) {
    long* k_row = shape[k];

    for (size_t i = 0; i < shape.s(); ++i) {
      long* i_row   = shape[i];
      long* k_row_l = k_row;

      long ik = i_row[k];
      for (size_t j = 0; j < shape.s(); ++j, ++i_row, ++k_row_l) {
        long distance = ik + *k_row_l;
        if (*i_row > distance)
          *i_row = distance;
      };
    };
  };
};

constexpr long
no_edge_value()
{
  return ((std::numeric_limits<long>::max)() / 2) - 1;
};

template<typename T>
class matrix_allocation_scope
{
private:
  T* m_mem;

public:
  matrix_allocation_scope()
    : m_mem(nullptr)
  {}
  ~matrix_allocation_scope()
  {
    if (this->m_mem != nullptr)
      delete[] this->m_mem;
  }

  square_shape<T> allocate(size_t vertex_count, size_t edge_count)
  {
    if (this->m_mem != nullptr)
      throw std::runtime_error("erro: can't reuse already used allocation scope");

    this->m_mem = new T[vertex_count * vertex_count];
    if (this->m_mem == nullptr) {
      throw std::runtime_error("erro: can't allocate memory");
    }

    square_shape<T> shape(this->m_mem, vertex_count);

    std::fill(shape.begin(), shape.end(), no_edge_value());

    return shape;
  }
};

template<typename T>
class matrix_graph_output : public graph_output<square_shape<T>, matrix_allocation_scope<T>>
{
public:
  matrix_graph_output(matrix_allocation_scope<T>& allocation_scope)
    : graph_output(allocation_scope)
  {}
};

int
main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << ""
              << "Options:\n"
              << "\t-i,--input\t\tFull path to input file in dimacs9 format"
              << std::endl;
    return 1;
  }

  std::string input_path;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-i") {
      if (i == (argc - 1)) {
        std::cerr << "No path has been specified for '-i' command" << std::endl;
        return 1;
      }
      input_path = argv[2];
    }
  }

  matrix_allocation_scope<long> scope;
  matrix_graph_output<long>     output(scope);

  scan_graph_from_dimacs9_file<long>(input_path, output);

  square_shape<long> shape = output();

  _impl(shape);

  for (size_t i = 0; i < shape.s(); ++i)
    for (size_t j = 0; j < shape.s(); ++j)
      if (shape[i][j] != no_edge_value())
        std::cout << "m[" << i << "," << j << "] = " << shape[i][j] << endl;

  return 0;
}

#pragma once

#include "memory.hpp"
#include "constants.hpp"
#include "communities-io.hpp"
#include "graphs-io.hpp"

#include "matrix.hpp"
#include "matrix-manip.hpp"
#include "matrix-traits.hpp"
#include "matrix-abstract.hpp"
#include "matrix-access.hpp"

namespace utilz {
namespace matrices {
namespace io {

template<typename S>
class scan_matrix_params
{
  static_assert(false, "The matrix is not supported");
};

template<typename T, typename A>
class scan_matrix_params<square_matrix<T, A>>
{
public:
  using matrix_type = square_matrix<T, A>;
  using size_type   = typename traits::matrix_traits<matrix_type>::size_type;
  using value_type  = typename traits::matrix_traits<matrix_type>::value_type;

  using graph_type       = std::tuple<size_type, std::vector<std::tuple<size_type, size_type, value_type>>>;
  using graph_reference  = graph_type&;

  using buffer_type      = memory::buffer;
  using buffer_reference = buffer_type&;

private:
  buffer_reference m_buffer;
  graph_reference  m_graph;

public:
  scan_matrix_params(buffer_reference buffer, graph_reference graph)
    : m_buffer(buffer)
    , m_graph(graph)
  {
  }

  buffer_reference
  buffer()
  {
    return this->m_buffer;
  }

  graph_reference
  graph()
  {
    return this->m_graph;
  }
};

template<typename T, typename A, typename U>
class scan_matrix_params<square_matrix<square_matrix<T, A>, U>>
{
public:
  using matrix_type = square_matrix<square_matrix<T, A>, U>;
  using size_type   = typename traits::matrix_traits<matrix_type>::size_type;
  using value_type  = typename traits::matrix_traits<matrix_type>::value_type;

  using graph_type       = std::tuple<size_type, std::vector<std::tuple<size_type, size_type, value_type>>>;
  using graph_reference  = graph_type&;

  using buffer_type      = memory::buffer;
  using buffer_reference = buffer_type&;

private:
  buffer_reference m_buffer;
  graph_reference  m_graph;

  size_type m_block_size;

public:
  scan_matrix_params(buffer_reference buffer, graph_reference graph, size_type block_size)
    : m_buffer(buffer)
    , m_graph(graph)
    , m_block_size(block_size)
  {
  }

  buffer_reference
  buffer()
  {
    return this->m_buffer;
  }

  graph_reference
  graph()
  {
    return this->m_graph;
  }

  size_type
  block_size()
  {
    return this->m_block_size;
  }
};

template<typename T, typename A, typename U>
class scan_matrix_params<square_matrix<rect_matrix<T, A>, U>>
{
public:
  using matrix_type = square_matrix<square_matrix<T, A>, U>;
  using size_type   = typename traits::matrix_traits<matrix_type>::size_type;
  using value_type  = typename traits::matrix_traits<matrix_type>::value_type;

  using graph_type       = std::tuple<size_type, std::vector<std::tuple<size_type, size_type, value_type>>>;
  using graph_reference  = graph_type&;

  using communities_type      = typename std::map<size_type, std::vector<size_type>>;
  using communities_reference = communities_type&;

  using buffer_type      = memory::buffer;
  using buffer_reference = buffer_type&;

private:
  buffer_reference      m_buffer;
  graph_reference       m_graph;
  communities_reference m_communities;

public:
  scan_matrix_params(buffer_reference buffer, graph_reference graph, communities_reference communities)
    : m_buffer(buffer)
    , m_graph(graph)
    , m_communities(communities)
  {
  }

  buffer_reference
  buffer()
  {
    return this->m_buffer;
  }

  graph_reference
  graph()
  {
    return this->m_graph;
  }

  communities_reference
  communities()
  {
    return this->m_communities;
  }
};

template<typename abstract_type>
void
print_matrix(
  utilz::graphs::io::graph_format format,
  std::ostream&                   os,
  abstract_type&                  abstract);

template<typename T, typename A>
void
scan_init_matrix(
  square_matrix<T, A>& matrix,
  scan_matrix_params<square_matrix<T, A>> params)
{
  using matrix_type       = square_matrix<T, A>;
  using size_type         = typename matrix_type::size_type;

  using graph_reference   = typename scan_matrix_params<matrix_type>::graph_reference;
  using buffer_reference  = typename scan_matrix_params<matrix_type>::buffer_reference;

  graph_reference  graph  = params.graph();
  buffer_reference buffer = params.buffer();

  size_type vc;
  std::tie(vc, std::ignore)             = graph;
  memory::buffer_allocator<T> allocator = memory::buffer_allocator<T>(&buffer);

  matrix = square_matrix<T, A>(vc, allocator);
};

template<typename T, typename A, typename U>
void
scan_init_matrix(
  square_matrix<square_matrix<T, A>, U>& matrix,
  scan_matrix_params<square_matrix<square_matrix<T, A>, U>> params)
{
  using matrix_block_type = square_matrix<T, A>;
  using matrix_type       = square_matrix<matrix_block_type, U>;
  using size_type         = typename matrix_type::size_type;

  using graph_reference   = typename scan_matrix_params<matrix_type>::graph_reference;
  using buffer_reference  = typename scan_matrix_params<matrix_type>::buffer_reference;

  graph_reference  graph  = params.graph();
  buffer_reference buffer = params.buffer();

  size_type vc;
  std::tie(vc, std::ignore) = graph;

  auto block_allocator  = memory::buffer_allocator<T>(&buffer);
  auto matrix_allocator = memory::buffer_allocator<matrix_block_type>(&buffer);

  auto block_size  = params.block_size();
  auto matrix_size = vc / block_size;
  if (vc % block_size != size_type(0))
    ++matrix_size;

  matrix = matrix_type(matrix_size, matrix_allocator);

  for (auto i = size_type(0); i < matrix.size(); ++i) {
    for (auto j = size_type(0); j < matrix.size(); ++j) {
      matrix.at(i, j) = matrix_block_type(block_size, block_allocator);
    }
  }
};

template<typename T, typename A, typename U>
void
scan_init_matrix(
  square_matrix<rect_matrix<T, A>, U>& matrix,
  scan_matrix_params<square_matrix<rect_matrix<T, A>, U>> params)
{
  using matrix_block_type = rect_matrix<T, A>;
  using matrix_type       = square_matrix<matrix_block_type, U>;
  using size_type         = typename matrix_type::size_type;

  using communities_reference = typename scan_matrix_params<matrix_type>::communities_reference;
  using buffer_reference      = typename scan_matrix_params<matrix_type>::buffer_reference;

  communities_reference communities  = params.communities();
  buffer_reference      buffer       = params.buffer();

  auto block_allocator  = memory::buffer_allocator<T>(&buffer);
  auto matrix_allocator = memory::buffer_allocator<matrix_block_type>(&buffer);

  std::vector<size_type> rect_sizes;
  for (auto [c, v] : communities)
      rect_sizes.push_back(v.size());

  matrix = matrix_type(rect_sizes.size(), matrix_allocator);

  for (auto i = size_type(0); i < matrix.size(); ++i)
    for (auto j = size_type(0); j < matrix.size(); ++j)
      matrix.at(i, j) = matrix_block_type(rect_sizes[j], rect_sizes[i], block_allocator);
};

template<access::matrix_access_schema TSchema, typename S>
void
scan_set_matrix(
  access::matrix_access<TSchema, S>& matrix_access,
  scan_matrix_params<S> params)
{
  using value_type = typename traits::matrix_traits<S>::value_type;

  matrix_access.set_all(utilz::constants::infinity<value_type>());

  auto [vc, edges] = params.graph();

  for (auto [f, t, w] : edges)
    matrix_access.at(f, t) = w;

  matrix_access.set_diagonal(value_type(0));
};

template<typename T, typename A, typename U>
void
scan_matrix_clusters(
  clusters& matrix_clusters,
  scan_matrix_params<square_matrix<rect_matrix<T, A>, U>> params)
{
  auto [vc, edges] = params.graph();

  for (auto [c, v] : params.communities())
    for (auto i : v)
      matrix_clusters.insert_vertex(c, i);

  for (auto [f, t, w] : edges)
    matrix_clusters.insert_edge(f, t);
}

template<access::matrix_access_schema TSchema, typename S>
void
print_matrix(
  utilz::graphs::io::graph_format    format,
  std::ostream&                      os,
  access::matrix_access<TSchema, S>& matrix_access)
{
  using size_type  = typename traits::matrix_traits<S>::size_type;
  using value_type = typename traits::matrix_traits<S>::value_type;

  auto dimensions = matrix_access.dimensions();

  std::vector<std::tuple<size_type, size_type, value_type>> edges;
  for (auto i = size_type(0); i < dimensions.h(); ++i) {
    for (auto j = size_type(0); j < dimensions.w(); ++j) {
      if (i == j)
        continue;

      auto value = matrix_access.at(i, j);
      if (value != utilz::constants::infinity<value_type>())
        edges.push_back(std::make_tuple(i, j, value));
    }
  }

  utilz::graphs::io::print_graph(format, os, dimensions.max(), edges);
};

} // namespace io
} // namespace matrix
} // namespace utilz

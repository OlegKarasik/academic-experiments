#pragma once

#include "communities-io.hpp"
#include "constants.hpp"
#include "graphs-io.hpp"

#include "matrix-manip.hpp"
#include "matrix-traits.hpp"
#include "matrix.hpp"
#include "matrix-abstract.hpp"

namespace utilz {
namespace matrices {
namespace io {

template<typename T, typename A>
void
scan_matrix(
  utilz::graphs::io::graph_format       format,
  std::istream&                         is,
  matrix_abstract<square_matrix<T, A>>& abstract);

template<typename T, typename A, typename U>
void
scan_matrix(
  utilz::graphs::io::graph_format                           format,
  std::istream&                                             is,
  matrix_abstract<square_matrix<square_matrix<T, A>, U>>&   abstract,
  typename square_matrix<square_matrix<T, A>, U>::size_type block_size);

template<typename T, typename A, typename U>
void
scan_matrix(
  utilz::graphs::io::graph_format                       graph_format,
  std::istream&                                         graph_is,
  utilz::communities::io::communities_format            communities_format,
  std::istream&                                         communities_is,
  matrix_abstract<square_matrix<rect_matrix<T, A>, U>>& abstract,
  clusters&                                             clusters);

template<typename abstract_type>
void
print_matrix(
  utilz::graphs::io::graph_format format,
  std::ostream&                   os,
  abstract_type&                  abstract);

template<typename T, typename A>
void
scan_matrix(
  utilz::graphs::io::graph_format       format,
  std::istream&                         is,
  matrix_abstract<square_matrix<T, A>>& abstract)
{
  static_assert(traits::matrix_traits<T>::is_type::value, "erro: unexpected matrix of matrices type");

  using abstract_type = matrix_abstract<square_matrix<T, A>>;
  using size_type     = typename abstract_type::size_type;
  using value_type    = typename abstract_type::value_type;

  using abstract_set_all      = procedures::abstract_set_all<abstract_type>;
  using abstract_set_diagonal = procedures::abstract_set_diagonal<abstract_type>;

  abstract_set_all      set_all;
  abstract_set_diagonal set_diagonal;

  auto [vc, edges] = utilz::graphs::io::scan_graph<size_type, value_type>(format, is);

  auto& matrix = abstract.matrix();
        matrix = square_matrix<T, A>(vc, matrix.get_allocator());

  abstract.rebind();

  set_all(abstract, utilz::constants::infinity<value_type>());

  for (auto [f, t, w] : edges)
    abstract.at(f, t) = w;

  set_diagonal(abstract, value_type(0));
};

template<typename T, typename A, typename U>
void
scan_matrix(
  utilz::graphs::io::graph_format                           format,
  std::istream&                                             is,
  matrix_abstract<square_matrix<square_matrix<T, A>, U>>&   abstract,
  typename square_matrix<square_matrix<T, A>, U>::size_type block_size)
{
  static_assert(traits::matrix_traits<T>::is_type::value, "erro: unexpected matrix of matrices of matrices type");

  using abstract_type  = matrix_abstract<square_matrix<square_matrix<T, A>, U>>;
  using size_type      = typename abstract_type::size_type;
  using value_type     = typename abstract_type::value_type;

  using abstract_set_all      = procedures::abstract_set_all<abstract_type>;
  using abstract_set_diagonal = procedures::abstract_set_diagonal<abstract_type>;

  abstract_set_all      set_all;
  abstract_set_diagonal set_diagonal;

  auto [vc, edges] = utilz::graphs::io::scan_graph<size_type, value_type>(format, is);

  auto size = vc / block_size;
  if (vc % block_size != size_type(0))
    ++size;

  auto& matrix = abstract.matrix();
        matrix = square_matrix<square_matrix<T, A>, U>(size, matrix.get_allocator());

  for (auto i = size_type(0); i < matrix.size(); ++i) {
    for (auto j = size_type(0); j < matrix.size(); ++j) {
      typename std::allocator_traits<U>::template rebind_alloc<A> allocator(matrix.get_allocator());

      matrix.at(i, j) = square_matrix<T, A>(block_size, allocator);
    }
  }

  abstract.rebind();

  set_all(abstract, utilz::constants::infinity<value_type>());

  for (auto [f, t, w] : edges)
    abstract.at(f, t) = w;

  set_diagonal(abstract, value_type(0));
};

template<typename T, typename A, typename U>
void
scan_matrix(
  utilz::graphs::io::graph_format                       graph_format,
  std::istream&                                         graph_is,
  utilz::communities::io::communities_format            communities_format,
  std::istream&                                         communities_is,
  matrix_abstract<square_matrix<rect_matrix<T, A>, U>>& abstract,
  clusters&                                             clusters)
{
  static_assert(traits::matrix_traits<T>::is_type::value, "erro: unexpected matrix of matrices or matrices type");

  using abstract_type  = matrix_abstract<square_matrix<rect_matrix<T, A>, U>>;
  using size_type      = typename abstract_type::size_type;
  using value_type     = typename abstract_type::value_type;
  using clusters_type  = utilz::matrices::clusters;

  using abstract_set_all      = procedures::abstract_set_all<abstract_type>;
  using abstract_set_diagonal = procedures::abstract_set_diagonal<abstract_type>;

  abstract_set_all      set_all;
  abstract_set_diagonal set_diagonal;

  auto [vc, edges] = utilz::graphs::io::scan_graph<size_type, value_type>(graph_format, graph_is);

  auto communities = utilz::communities::io::scan_communities<size_type>(communities_format, communities_is);
  for (auto [c, v] : communities) {
    for (auto i : v) {
      clusters.insert_vertex(c, i);
    }
  }

  std::vector<size_type> item_sizes;
  for (auto size : clusters.list() | std::views::transform([](auto& group) -> auto { return group.size(); }))
    item_sizes.push_back(size);

  auto& matrix = abstract.matrix();
        matrix = square_matrix<rect_matrix<T, A>, U>(item_sizes.size(), matrix.get_allocator());

  for (auto i = size_type(0); i < matrix.size(); ++i) {
    for (auto j = size_type(0); j < matrix.size(); ++j) {
      typename std::allocator_traits<U>::template rebind_alloc<A> allocator(matrix.get_allocator());

      matrix.at(i, j) = rect_matrix<T, A>(item_sizes[j], item_sizes[i], allocator);
    }
  }

  abstract.rebind();

  set_all(abstract, utilz::constants::infinity<value_type>());

  for (auto [f, t, w] : edges) {
    clusters.insert_edge(f, t);
    abstract.at(f, t) = w;
  }

  set_diagonal(abstract, value_type(0));
};

template<typename abstract_type>
void
print_matrix(
  utilz::graphs::io::graph_format format,
  std::ostream&                   os,
  abstract_type&                  abstract)
{
  using size_type  = typename abstract_type::size_type;
  using value_type = typename abstract_type::value_type;

  auto size = abstract.size();

  std::vector<std::tuple<size_type, size_type, value_type>> edges;
  for (auto i = size_type(0); i < size; ++i) {
    for (auto j = size_type(0); j < size; ++j) {
      auto value = abstract.at(i, j);
      if (value != utilz::constants::infinity<value_type>())
        edges.push_back(std::make_tuple(i, j, value));
    }
  }

  utilz::graphs::io::print_graph(format, os, size, edges);
};

} // namespace io
} // namespace matrix
} // namespace utilz

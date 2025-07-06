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

template<typename T, typename A>
void
print_matrix(
  utilz::graphs::io::graph_format format,
  std::ostream&                   os,
  square_matrix<T, A>&            matrix);

namespace impl {

template<typename T>
class iterator;

} // namespace impl

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

  using abstract_set_all = procedures::abstract_set_all<abstract_type>;

  abstract_set_all set_all;

  auto set_vc = std::function([&set_all](abstract_type& abstract, size_type vertex_count) -> void {
    auto& matrix = abstract.matrix();
          matrix = square_matrix<T, A>(vertex_count, matrix.get_allocator());

    abstract.rebind();

    set_all(abstract, utilz::constants::infinity<value_type>());
  });
  auto set_ec = std::function([](abstract_type& abstract, size_type edge_count) -> void {});
  auto set_wv = std::function([](abstract_type& abstract, size_type f, size_type t, value_type w) -> void {
    abstract.at(f, t) = w;
  });

  utilz::graphs::io::scan_graph(format, is, abstract, set_vc, set_ec, set_wv);

  auto dimensions = abstract.dimensions();
  for (auto i = size_type(0); i < dimensions.s(); ++i)
    abstract.at(i, i) = value_type(0);
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

  using abstract_set_all = procedures::abstract_set_all<abstract_type>;

  abstract_set_all set_all;

  auto set_vc = std::function([&set_all, &block_size](abstract_type& abstract, size_type vertex_count) -> void {
    auto size = vertex_count / block_size;
    if (vertex_count % block_size != size_type(0))
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
  });
  auto set_ec = std::function([](abstract_type& abstract, size_type edge_count) -> void {});
  auto set_wv = std::function([](abstract_type& abstract, size_type f, size_type t, value_type w) -> void {
    abstract.at(f, t) = w;
  });

  utilz::graphs::io::scan_graph(format, is, abstract, set_vc, set_ec, set_wv);

  auto dimensions = abstract.dimensions();
  for (auto i = size_type(0); i < dimensions.s(); ++i)
    abstract.at(i, i) = value_type(0);
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

  using abstract_set_all = procedures::abstract_set_all<abstract_type>;

  abstract_set_all set_all;

  auto set_cluster_value = std::function([](clusters_type& c, size_type cluster_idx, size_type vertex_idx) -> void {
    c.insert_vertex(cluster_idx, vertex_idx);
  });
  utilz::communities::io::scan_communities(communities_format, communities_is, clusters, set_cluster_value);

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

  auto set_matrix_value = std::function([&clusters](abstract_type& abstract, size_type f, size_type t, value_type w) -> void {
    clusters.insert_edge(f, t);

    abstract.at(f, t) = w;
  });
  utilz::graphs::io::scan_graph(graph_format, graph_is, abstract, set_matrix_value);

  auto dimensions = abstract.dimensions();
  for (auto i = size_type(0); i < dimensions.s(); ++i)
    abstract.at(i, i) = value_type(0);
};

template<typename T, typename A>
void
print_matrix(
  utilz::graphs::io::graph_format       format,
  std::ostream&                         os,
  utilz::matrices::square_matrix<T, A>& matrix)
{
  using iterator_type = typename utilz::matrices::io::impl::iterator<utilz::matrices::square_matrix<T, A>>;
  using value_type    = typename utilz::matrices::traits::matrix_traits<utilz::matrices::square_matrix<T, A>>::value_type;

  typename utilz::matrices::procedures::matrix_at<utilz::matrices::square_matrix<T, A>>::bindable get_at;

  get_at.bind(matrix);

  auto get_iterators = std::function([&get_at](utilz::matrices::square_matrix<T, A>& c) -> std::tuple<iterator_type, iterator_type> {
    auto begin = iterator_type(c, get_at, utilz::constants::infinity<value_type>(), typename iterator_type::begin_iterator());
    auto end   = iterator_type(c, utilz::constants::infinity<value_type>(), typename iterator_type::end_iterator());
    return std::make_tuple(begin, end);
  });

  utilz::graphs::io::print_graph(format, os, matrix, get_iterators);
};

namespace impl {

template<typename S>
class iterator
{
  static_assert(utilz::matrices::traits::matrix_traits<S>::is_matrix::value, "erro: input type has to be a square_matrix");

private:
  using _size_type   = typename utilz::matrices::traits::matrix_traits<S>::size_type;
  using _value_type  = typename utilz::matrices::traits::matrix_traits<S>::value_type;

  using _get_dimensions_type = typename utilz::matrices::procedures::matrix_get_dimensions<S>;
  using _get_at_type         = typename utilz::matrices::procedures::matrix_at<S>::bindable;

public:
  // Iterator definitions
  //
  using iterator_category = std::forward_iterator_tag;
  using difference_type   = _size_type;
  using value_type        = std::tuple<_size_type, _size_type, _value_type>;
  using pointer           = value_type*;
  using reference         = value_type&;

private:
  _size_type m_width;
  _size_type m_height;
  _size_type m_i;
  _size_type m_j;

  _value_type m_infinity;

  S& m_s;

  _get_dimensions_type m_get_dimensions;
  _get_at_type         m_get_at;

public:
  struct begin_iterator
  {
  };
  struct end_iterator
  {
  };

public:
  iterator(S& s, _get_at_type& get_at, _value_type infinity, begin_iterator)
    : m_s(s)
    , m_get_at(get_at)
  {
    auto dimensions = this->m_get_dimensions(s);

    this->m_width  = dimensions.w();
    this->m_height = dimensions.h();
    this->m_i      = _size_type(0);
    this->m_j      = _size_type(0);

    this->m_infinity = infinity;

    if (!s.empty())
      ++(*this);
  }

  iterator(S& s, _value_type infinity, end_iterator)
    : m_s(s)
  {
    auto dimensions = this->m_get_dimensions(s);

    this->m_width  = dimensions.w();
    this->m_height = dimensions.h();
    this->m_i      = _size_type(this->m_height);
    this->m_j      = _size_type(this->m_width);

    this->m_infinity = infinity;
  }

  value_type
  operator*()
  {
    return std::make_tuple(this->m_i, this->m_j, this->m_get_at(this->m_s, this->m_i, this->m_j));
  }

  iterator&
  operator++()
  {
    do {
      if (++this->m_j == this->m_width) {
        this->m_j = _size_type(0);
        if (++this->m_i == this->m_height) {
          this->m_i = this->m_height;
          this->m_j = this->m_width;

          break;
        }
      }
    } while (this->m_i == this->m_j || this->m_get_at(this->m_s, this->m_i, this->m_j) == this->m_infinity);

    return *this;
  }

  iterator
  operator++(int)
  {
    auto v = *this;

    ++(*this);

    return v;
  }

  friend bool
  operator==(const iterator& a, const iterator& b)
  {
    return a.m_s == b.m_s && a.m_infinity == b.m_infinity && a.m_i == b.m_i && a.m_j == b.m_j && a.m_width == b.m_width && a.m_height == b.m_height;
  };

  friend bool
  operator!=(const iterator& a, const iterator& b)
  {
    return !(a == b);
  };
};

} // namespace impl

} // namespace io
} // namespace matrix
} // namespace utilz

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
  utilz::graphs::io::graph_format       format,
  std::ostream&                         os,
  matrix_abstract<square_matrix<T, A>>& abstract);

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

  using abstract_set_all      = procedures::abstract_set_all<abstract_type>;
  using abstract_set_diagonal = procedures::abstract_set_diagonal<abstract_type>;

  abstract_set_all      set_all;
  abstract_set_diagonal set_diagonal;

  auto [vc, ec, edges] = utilz::graphs::io::scan_graph<size_type, value_type>(format, is);

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

  auto [vc, ec, edges] = utilz::graphs::io::scan_graph<size_type, value_type>(format, is);

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

  auto [vc, ec, edges] = utilz::graphs::io::scan_graph<size_type, value_type>(graph_format, graph_is);

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

template<typename T, typename A>
void
print_matrix(
  utilz::graphs::io::graph_format       format,
  std::ostream&                         os,
  matrix_abstract<square_matrix<T, A>>& abstract)
{
  using abstract_type = matrix_abstract<square_matrix<T, A>>;
  using iterator_type = typename io::impl::iterator<abstract_type>;
  using value_type    = typename abstract_type::value_type;

  auto get_iterators = std::function([](abstract_type& abstract) -> std::tuple<iterator_type, iterator_type> {
    auto begin = iterator_type(abstract, utilz::constants::infinity<value_type>(), typename iterator_type::begin_iterator());
    auto end   = iterator_type(abstract, utilz::constants::infinity<value_type>(), typename iterator_type::end_iterator());
    return std::make_tuple(begin, end);
  });

  utilz::graphs::io::print_graph(format, os, abstract, get_iterators);
};

namespace impl {

template<typename S>
class iterator
{
private:
  using _size_type   = typename S::size_type;
  using _value_type  = typename S::value_type;

public:
  // Iterator definitions
  //
  using iterator_category = std::forward_iterator_tag;
  using difference_type   = _size_type;
  using value_type        = std::tuple<_size_type, _size_type, _value_type>;
  using pointer           = value_type*;
  using reference         = value_type&;

private:
  _size_type m_i;
  _size_type m_j;

  _value_type m_infinity;

  S& m_s;

public:
  struct begin_iterator
  {
  };
  struct end_iterator
  {
  };

public:
  iterator(S& s, _value_type infinity, begin_iterator)
    : m_s(s)
  {
    this->m_i = _size_type(0);
    this->m_j = _size_type(0);

    this->m_infinity = infinity;

    if (s.w() != _size_type(0) && s.h() != _size_type(0))
      ++(*this);
  }

  iterator(S& s, _value_type infinity, end_iterator)
    : m_s(s)
  {
    this->m_i = _size_type(s.h());
    this->m_j = _size_type(s.w());

    this->m_infinity = infinity;
  }

  value_type
  operator*()
  {
    return std::make_tuple(this->m_i, this->m_j, this->m_s.at(this->m_i, this->m_j));
  }

  iterator&
  operator++()
  {
    do {
      if (++this->m_j == this->m_s.w()) {
        this->m_j = _size_type(0);
        if (++this->m_i == this->m_s.h()) {
          this->m_i = this->m_s.h();
          this->m_j = this->m_s.w();

          break;
        }
      }
    } while (this->m_i == this->m_j || this->m_s.at(this->m_i, this->m_j) == this->m_infinity);

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
    return a.m_infinity == b.m_infinity && a.m_i == b.m_i && a.m_j == b.m_j;
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

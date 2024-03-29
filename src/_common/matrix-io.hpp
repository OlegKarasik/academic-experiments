#pragma once

#include "communities-io.hpp"
#include "constants.hpp"
#include "graphs-io.hpp"

#include "matrix-manip.hpp"
#include "matrix-traits.hpp"
#include "matrix.hpp"

namespace utilz {

namespace communities {
namespace io {

template<typename I>
void
scan_communities(
  communities_format         format,
  std::istream&              is,
  utilz::matrix_clusters<I>& clusters);

template<typename I>
void
scan_communities(
  communities_format         format,
  std::istream&              is,
  utilz::matrix_clusters<I>& clusters)
{
  auto set_v = std::function([](utilz::matrix_clusters<I>& c, I cindex, I vindex) -> void {
    c.insert(cindex, vindex);
  });

  utilz::communities::io::scan_communities(format, is, clusters, set_v);
};

} // namespace io
} // namespace communities

namespace graphs {
namespace io {

template<typename T, typename A, typename... TArgs>
void
scan_graph(
  graph_format                format,
  std::istream&               is,
  utilz::square_matrix<T, A>& matrix,
  TArgs... item_sizes);

template<typename T, typename A>
void
scan_graph(
  graph_format                                                                                          format,
  std::istream&                                                                                         is,
  utilz::square_matrix<T, A>&                                                                           matrix,
  utilz::matrix_clusters<typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type>& clusters);

template<typename T, typename A>
void
print_graph(
  graph_format                format,
  std::ostream&               os,
  utilz::square_matrix<T, A>& matrix);

namespace impl {

template<typename T>
class iterator;

} // namespace impl

template<typename T, typename A, typename... TArgs>
void
scan_graph(
  graph_format                format,
  std::istream&               is,
  utilz::square_matrix<T, A>& matrix,
  TArgs... item_sizes)
{
  using matrix_set_dimensions = utilz::procedures::matrix_set_dimensions<utilz::square_matrix<T, A>>;
  using matrix_get_dimensions = utilz::procedures::matrix_get_dimensions<utilz::square_matrix<T, A>>;
  using matrix_at             = utilz::procedures::matrix_at<utilz::square_matrix<T, A>>;

  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::value_type;

  matrix_set_dimensions set_dimensions;
  matrix_get_dimensions get_dimensions;
  matrix_at             get_at;

  auto ss_fn = std::function([&set_dimensions, &item_sizes...](utilz::square_matrix<T, A>& c, size_type vertex_count) -> void {
    set_dimensions(c, vertex_count, item_sizes...);
  });
  auto se_fn = std::function([](utilz::square_matrix<T, A>& c, size_type edge_count) -> void {
  });
  auto sw_fn = std::function([&get_at](utilz::square_matrix<T, A>& c, size_type f, size_type t, value_type w) -> void {
    get_at(c, f, t) = w;
  });

  utilz::graphs::io::scan_graph(format, is, matrix, ss_fn, se_fn, sw_fn);

  auto dimensions = get_dimensions(matrix);

  for (auto i = size_type(0); i < dimensions.s(); ++i)
    for (auto j = size_type(0); j < dimensions.s(); ++j) {
      auto v = get_at(matrix, i, j);
      if (v == value_type(0))
        get_at(matrix, i, j) = utilz::constants::infinity<value_type>();

      if (i == j)
        get_at(matrix, i, j) = value_type(0);
    }
};

template<typename T, typename A>
void
scan_graph(
  graph_format                                                                                          format,
  std::istream&                                                                                         is,
  utilz::square_matrix<T, A>&                                                                           matrix,
  utilz::matrix_clusters<typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type>& clusters)
{
  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::value_type;

  using matrix_clusters = utilz::matrix_clusters<size_type>;

  using matrix_set_dimensions = utilz::procedures::matrix_set_dimensions<utilz::square_matrix<T, A>>;
  using matrix_get_dimensions = utilz::procedures::matrix_get_dimensions<utilz::square_matrix<T, A>>;
  using matrix_at             = utilz::procedures::matrix_at<utilz::square_matrix<T, A>>;
  using matrix_replace        = utilz::procedures::matrix_replace<utilz::square_matrix<T, A>>;

  matrix_set_dimensions set_dimensions;
  matrix_get_dimensions get_dimensions;
  matrix_at             get_at;
  matrix_replace        replace_all;

  std::vector<size_type> item_sizes;
  for (auto size : clusters.list_clusters() | std::views::transform([&clusters](auto& cindex) -> typename matrix_clusters::size_type { return clusters.count_vertices(cindex); }))
    item_sizes.push_back(size);

  set_dimensions(matrix, item_sizes);

  auto sw_fn = std::function([&get_at](utilz::square_matrix<T, A>& c, size_type f, size_type t, value_type w) -> void {
    get_at(c, f, t) = w;
  });

  utilz::graphs::io::scan_graph(format, is, matrix, sw_fn);

  auto dimensions = get_dimensions(matrix);

  for (auto i = size_type(0); i < dimensions.s(); ++i)
    for (auto j = size_type(0); j < dimensions.s(); ++j) {
      auto v = get_at(matrix, i, j);
      if (v == value_type(0))
        get_at(matrix, i, j) = utilz::constants::infinity<value_type>();

      if (i == j)
        get_at(matrix, i, j) = value_type(0);
    }

  for (auto i = size_type(0); i < dimensions.s(); ++i)
    for (auto j = size_type(0); j < dimensions.s(); ++j) {
      if (get_at(matrix, i, j) != utilz::constants::infinity<value_type>()) {
        auto edge = std::make_pair(i, j);
        clusters.insert(edge);
      }
    }
};

template<typename T, typename A>
void
print_graph(
  graph_format                format,
  std::ostream&               os,
  utilz::square_matrix<T, A>& matrix)
{
  static_assert(utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::is_matrix::value, "erro: input type has to be a square_matrix");

  using iter_type  = typename utilz::graphs::io::impl::iterator<utilz::square_matrix<T, A>>;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::value_type;

  auto gt_fn = std::function([](utilz::square_matrix<T, A>& c) -> std::tuple<iter_type, iter_type> {
    auto begin = iter_type(c, utilz::constants::infinity<value_type>(), typename iter_type::begin_iterator());
    auto end   = iter_type(c, utilz::constants::infinity<value_type>(), typename iter_type::end_iterator());
    return std::make_tuple(begin, end);
  });

  utilz::graphs::io::print_graph(format, os, matrix, gt_fn);
};

namespace impl {

template<typename S>
class iterator
{
  static_assert(utilz::traits::matrix_traits<S>::is_matrix::value, "erro: input type has to be a square_matrix");

private:
  using _size_type  = typename utilz::traits::matrix_traits<S>::size_type;
  using _value_type = typename utilz::traits::matrix_traits<S>::value_type;

public:
  // Iterator definitions
  //
  using iterator_category = std::forward_iterator_tag;
  using difference_type   = _size_type;
  using value_type        = typename std::tuple<_size_type, _size_type, _value_type>;
  using pointer           = value_type*;
  using reference         = value_type&;

private:
  _size_type m_width;
  _size_type m_height;
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
    utilz::procedures::matrix_get_dimensions<S> get_dimensions;

    auto dimensions = get_dimensions(s);

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
    utilz::procedures::matrix_get_dimensions<S> get_dimensions;

    auto dimensions = get_dimensions(s);

    this->m_width  = dimensions.w();
    this->m_height = dimensions.h();
    this->m_i      = _size_type(this->m_height);
    this->m_j      = _size_type(this->m_width);

    this->m_infinity = infinity;
  }

  value_type
  operator*()
  {
    utilz::procedures::matrix_at<S> get_at;

    return std::make_tuple(this->m_i, this->m_j, get_at(this->m_s, this->m_i, this->m_j));
  }

  iterator&
  operator++()
  {
    utilz::procedures::matrix_at<S> get_at;

    do {
      if (++this->m_j == this->m_width) {
        this->m_j = _size_type(0);
        if (++this->m_i == this->m_height) {
          this->m_i = this->m_height;
          this->m_j = this->m_width;

          break;
        }
      }
    } while (this->m_i == this->m_j || get_at(this->m_s, this->m_i, this->m_j) == this->m_infinity);

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
} // namespace graphs
} // namespace utilz

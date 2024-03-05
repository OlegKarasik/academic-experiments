#pragma once

#include "constants.hpp"
#include "graphs-io.hpp"

#include "matrix.hpp"
#include "matrix-manip.hpp"
#include "matrix-traits.hpp"

namespace utilz {
namespace graphs {
namespace io {

template<typename T, typename A, typename... TArgs>
void
scan_graph(
  graph_format                format,
  std::istream&               is,
  utilz::square_matrix<T, A>& matrix,
  TArgs... args);

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
  TArgs... args)
{
  using matrix_set_dimensions = utilz::procedures::matrix_set_dimensions<utilz::square_matrix<T, A>>;
  using matrix_at             = utilz::procedures::matrix_at<utilz::square_matrix<T, A>>;
  using matrix_replace        = utilz::procedures::matrix_replace<utilz::square_matrix<T, A>>;

  using size_type  = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using value_type = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::value_type;

  matrix_set_dimensions set_dimensions;
  matrix_at             get_at;
  matrix_replace        replace;

  auto ss_fn = std::function([&set_dimensions, &replace, &args...](utilz::square_matrix<T, A>& c, size_type vertex_count) -> void {
    set_dimensions(c, vertex_count, args...);
    replace(c, value_type(), utilz::constants::infinity<value_type>());
  });
  auto se_fn = std::function([](utilz::square_matrix<T, A>& c, size_type edge_count) -> void {
  });
  auto sw_fn = std::function([&get_at](utilz::square_matrix<T, A>& c, size_type f, size_type t, value_type w) -> void {
    get_at(c, f, t) = w;
  });

  utilz::graphs::io::scan_graph(format, is, matrix, ss_fn, se_fn, sw_fn);
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

    // If initial state of the iterator is infinity - then advance the iterator to first
    // non infinity value
    //
    if (!s.empty()) {
      utilz::procedures::matrix_at<S> get_at;
      if (get_at(s, this->m_i, this->m_j) == this->m_infinity)
        ++(*this);
    }
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
    } while (get_at(this->m_s, this->m_i, this->m_j) == this->m_infinity);

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

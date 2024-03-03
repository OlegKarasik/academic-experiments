#pragma once

#include "constants.hpp"
#include "graphs-io.hpp"

#include "square-shape.hpp"
#include "shapes-traits.hpp"
#include "shapes-manip.hpp"

namespace utilz {
namespace graphs {
namespace io {

template<typename S, typename... TArgs>
void
scan_graph(
  graph_format  format,
  std::istream& is,
  S&            shape,
  TArgs... args);

template<typename S>
void
print_graph(
  graph_format  format,
  std::ostream& os,
  S&            shape);

namespace shapes {

template<typename T>
class iterator;

} // namespace shapes

template<typename S, typename... TArgs>
void
scan_graph(
  graph_format  format,
  std::istream& is,
  S&            shape,
  TArgs... args)
{
  static_assert(utilz::traits::matrix_traits<S>::is_matrix::value, "erro: input type has to be a square_matrix");

  using SS = utilz::procedures::matrix_set_dimensions<S>;
  using SW = utilz::procedures::matrix_at<S>;
  using RP = utilz::procedures::matrix_replace<S>;

  using size_type  = typename utilz::traits::matrix_traits<S>::size_type;
  using value_type = typename utilz::traits::matrix_traits<S>::value_type;

  SS ss;
  SW sw;
  RP rp;

  auto ss_fn = std::function([&ss, &rp, &args...](S& c, size_type vertex_count) -> void {
    ss(c, vertex_count, args...);
    rp(c, value_type(), utilz::constants::infinity<value_type>());
  });
  auto se_fn = std::function([](S& c, size_type edge_count) -> void {
  });
  auto sw_fn = std::function([&sw](S& c, size_type f, size_type t, value_type w) -> void {
    sw(c, f, t) = w;
  });

  utilz::graphs::io::scan_graph(format, is, shape, ss_fn, se_fn, sw_fn);
};

template<typename S>
void
print_graph(
  graph_format  format,
  std::ostream& os,
  S&            shape)
{
  static_assert(utilz::traits::matrix_traits<S>::is_matrix::value, "erro: input type has to be a square_matrix");

  using iter_type  = typename utilz::graphs::io::shapes::iterator<S>;
  using value_type = typename utilz::traits::matrix_traits<S>::value_type;

  auto gt_fn = std::function([](S& c) -> std::tuple<iter_type, iter_type> {
    auto begin = iter_type(c, utilz::constants::infinity<value_type>(), typename iter_type::begin_iterator());
    auto end   = iter_type(c, utilz::constants::infinity<value_type>(), typename iter_type::end_iterator());
    return std::make_tuple(begin, end);
  });

  utilz::graphs::io::print_graph(format, os, shape, gt_fn);
};

namespace shapes {

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
    utilz::procedures::matrix_get_dimensions<S>  get_dimensions;

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
    utilz::procedures::matrix_get_dimensions<S>  get_dimensions;

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

} // namespace shapes

} // namespace io
} // namespace graphs
} // namespace utilz

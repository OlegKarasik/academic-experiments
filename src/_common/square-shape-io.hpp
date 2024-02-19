#pragma once

#include <iterator>
#include <memory>
#include <utility>

#include "square-shape.hpp"

namespace utilz {
namespace graphs {
namespace io {

// ---
// Forward declarations
//

template<typename T>
class square_shape_iterator;

//
// Forward declarations
// ---

template<typename T>
class square_shape_iterator
{
  static_assert(utilz::traits::square_shape_traits<T>::is::value, "erro: input type has to be a square_shape");

private:
  using _size_type  = typename utilz::traits::square_shape_traits<T>::size_type;
  using _value_type = typename utilz::traits::square_shape_traits<T>::value_type;

public:
  // Iterator definitions
  //
  using iterator_category = std::forward_iterator_tag;
  using difference_type   = _size_type;
  using value_type        = typename std::tuple<_size_type, _size_type, _value_type>;
  using pointer           = value_type*;
  using reference         = value_type&;

private:
  _size_type m_size;
  _size_type m_i;
  _size_type m_j;

  _value_type m_infinity;

  T& m_s;

public:
  struct begin_iterator
  {
  };
  struct end_iterator
  {
  };

public:
  square_shape_iterator(T& s, _value_type infinity, begin_iterator)
    : m_s(s)
  {
    utilz::procedures::square_shape_get_size<T> get_size;

    this->m_size = get_size(s);
    this->m_i    = _size_type(0);
    this->m_j    = _size_type(0);

    this->m_infinity = infinity;

    // If initial state of the iterator is infinity - then advance the iterator to first
    // non infinity value
    //
    if (!s.empty()) {
      utilz::procedures::square_shape_at<T> at;
      if (at(s, this->m_i, this->m_j) == this->m_infinity)
        ++(*this);
    }
  }

  square_shape_iterator(T& s, _value_type infinity, end_iterator)
    : m_s(s)
  {
    utilz::procedures::square_shape_get_size<T> get_size;

    this->m_size = get_size(s);
    this->m_i    = _size_type(this->m_size);
    this->m_j    = _size_type(this->m_size);

    this->m_infinity = infinity;
  }

  value_type
  operator*()
  {
    utilz::procedures::square_shape_at<T> at;

    return std::make_tuple(this->m_i, this->m_j, at(this->m_s, this->m_i, this->m_j));
  }

  square_shape_iterator&
  operator++()
  {
    utilz::procedures::square_shape_at<T> at;

    do {
      if (++this->m_j == this->m_size) {
        this->m_j = _size_type(0);
        if (++this->m_i == this->m_size) {
          this->m_i == this->m_size;
          this->m_j == this->m_size;

          break;
        }
      }
    } while (at(this->m_s, this->m_i, this->m_j) == this->m_infinity);

    return *this;
  }

  square_shape_iterator
  operator++(int)
  {
    auto v = *this;

    ++(*this);

    return v;
  }

  friend bool
  operator==(const square_shape_iterator& a, const square_shape_iterator& b)
  {
    return a.m_s == b.m_s && a.m_infinity == b.m_infinity && a.m_i == b.m_i && a.m_j == b.m_j;
  };

  friend bool
  operator!=(const square_shape_iterator& a, const square_shape_iterator& b)
  {
    return !(a == b);
  };
};

} // namespace io
} // namespace graphs
} // namespace utilz

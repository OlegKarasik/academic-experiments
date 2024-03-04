#pragma once

#include <numeric>

#include "rect-shape.hpp"
#include "square-shape.hpp"

#include "shapes-traits.hpp"

namespace utilz {
namespace procedures {

enum matrix_dimensions_variant
{
  matrix_dimensions_variant_rect   = 1,
  matrix_dimensions_variant_square = 2
};

// ---
// Forward declarations
//

namespace impl {

template<typename T>
struct matrix_dimensions;

template<typename S, class Enable = void>
struct impl_get_dimensions;

template<typename S, class Enable = void>
struct impl_at;

template<typename S>
struct impl_replace;

template<typename S>
struct impl_set_dimensions;

} // namespace impl

//
// Forward declarations
// ---

template<typename S>
using matrix_get_dimensions = impl::impl_get_dimensions<S>;

template<typename S>
using matrix_set_dimensions = impl::impl_set_dimensions<S>;

template<typename S>
using matrix_at = impl::impl_at<S>;

template<typename S>
using matrix_replace = impl::impl_replace<S>;

template<matrix_dimensions_variant TVariant, typename S>
class matrix_dimensions
{
  static_assert(false, "The variant is not supported");
};

template<typename T>
class matrix_dimensions<matrix_dimensions_variant::matrix_dimensions_variant_rect, T>
  : public impl::matrix_dimensions<T>
{
private:
  using size_type = T;

public:
  matrix_dimensions()
    : impl::matrix_dimensions<size_type>(size_type(0), size_type(0))
  {
  }

  matrix_dimensions(size_type width, size_type height)
    : impl::matrix_dimensions<size_type>(width, height)
  {
  }

  template<matrix_dimensions_variant X, typename U>
  matrix_dimensions(matrix_dimensions<X, U> dimensions)
    : impl::matrix_dimensions<size_type>(dimensions.w(), dimensions.h())
  {
  }

  matrix_dimensions
  operator+(const matrix_dimensions& x) const
  {
    return matrix_dimensions(this->w() + x.w(), this->h() + x.h());
  }

  matrix_dimensions
  operator-(const matrix_dimensions& x) const
  {
    return matrix_dimensions(this->w() - x.w(), this->h() - x.h());
  }

  matrix_dimensions
  operator*(const matrix_dimensions& x) const
  {
    return matrix_dimensions(this->w() * x.w(), this->h() * x.h());
  }

  matrix_dimensions
  operator/(const matrix_dimensions& x) const
  {
    return matrix_dimensions(this->w() / x.w(), this->h() / x.h());
  }
};

template<typename T>
class matrix_dimensions<matrix_dimensions_variant::matrix_dimensions_variant_square, T>
  : public impl::matrix_dimensions<T>
{
private:
  using size_type = T;

public:
  matrix_dimensions()
    : impl::matrix_dimensions<size_type>(size_type(0), size_type(0))
  {
  }

  matrix_dimensions(size_type size)
    : impl::matrix_dimensions<size_type>(size, size)
  {
  }

  template<typename U>
  matrix_dimensions(matrix_dimensions<matrix_dimensions_variant::matrix_dimensions_variant_square, U> dimensions)
    : impl::matrix_dimensions<size_type>(dimensions.s(), dimensions.s())
  {
  }

  template<typename U>
  matrix_dimensions(matrix_dimensions<matrix_dimensions_variant::matrix_dimensions_variant_rect, U> dimensions)
    : impl::matrix_dimensions<size_type>(dimensions.w(), dimensions.h())
  {
    if (dimensions.w() != dimensions.h())
      std::logic_error("erro: can't rebind non-square to square dimensions when width and height are different");
  }

  size_type
  s() const
  {
    return this->w();
  }

  matrix_dimensions
  operator+(const matrix_dimensions& x) const
  {
    return matrix_dimensions(this->s() + x.s());
  }

  matrix_dimensions
  operator-(const matrix_dimensions& x) const
  {
    return matrix_dimensions(this->s() - x.s());
  }

  matrix_dimensions
  operator*(const matrix_dimensions& x) const
  {
    return matrix_dimensions(this->s() * x.s());
  }

  matrix_dimensions
  operator/(const matrix_dimensions& x) const
  {
    return matrix_dimensions(this->s() / x.s());
  }
};

namespace impl {

template<typename T>
class matrix_dimensions
{
private:
  T m_w;
  T m_h;

public:
  matrix_dimensions(T width, T height)
    : m_w(width)
    , m_h(height)
  {
  }

  T
  w() const
  {
    return this->m_w;
  }

  T
  h() const
  {
    return this->m_h;
  }
};

template<typename T, class Enable>
struct impl_get_dimensions
{
  static_assert(false, "erro: input type has to be a matrix");
};

template<typename T, typename A>
struct impl_get_dimensions<utilz::rect_matrix<T, A>, typename std::enable_if<utilz::traits::matrix_traits<T>::is_type::value>::type>
{
private:
  using size_type = typename utilz::traits::matrix_traits<utilz::rect_matrix<T, A>>::size_type;

public:
  using dimension_type = utilz::procedures::matrix_dimensions<matrix_dimensions_variant_rect, size_type>;

public:
  dimension_type
  operator()(const utilz::rect_matrix<T, A>& s)
  {
    return dimension_type(s.width(), s.height());
  }
};

template<typename T, typename A>
struct impl_get_dimensions<utilz::square_matrix<T, A>, typename std::enable_if<utilz::traits::matrix_traits<T>::is_type::value>::type>
{
private:
  using size_type = typename utilz::square_matrix<T, A>::size_type;

public:
  using dimension_type = utilz::procedures::matrix_dimensions<matrix_dimensions_variant_square, size_type>;

public:
  dimension_type
  operator()(const utilz::square_matrix<T, A>& s)
  {
    return dimension_type(s.size());
  }
};

template<typename T, typename A>
struct impl_get_dimensions<utilz::square_matrix<T, A>, typename std::enable_if<utilz::traits::matrix_traits<T>::is_matrix::value>::type>
{
private:
  using size_type = typename utilz::square_matrix<T, A>::size_type;

public:
  using dimension_type = utilz::procedures::matrix_dimensions<matrix_dimensions_variant_square, size_type>;

public:
  dimension_type
  operator()(const utilz::square_matrix<T, A>& s)
  {
    impl_get_dimensions<T> get_dimensions;

    typename impl_get_dimensions<T>::dimension_type dimensions;
    for (auto i = size_type(0); i < s.size(); ++i)
      dimensions = dimensions + get_dimensions(s.at(i, i));

    return dimension_type(dimensions);
  }
};

template<typename S, class Enable>
struct impl_at
{
  static_assert(false, "erro: input type has to be a matrix");
};

template<template<typename, typename> typename S, typename T, typename A>
struct impl_at<S<T, A>, typename std::enable_if<utilz::traits::matrix_traits<T>::is_type::value>::type>
{
  static_assert(utilz::traits::matrix_traits<S<T, A>>::is_matrix::value, "erro: input type has to be a matrix");

private:
  using size_type       = typename utilz::traits::matrix_traits<S<T, A>>::size_type;
  using reference       = typename utilz::traits::matrix_traits<S<T, A>>::reference;
  using const_reference = typename utilz::traits::matrix_traits<S<T, A>>::const_reference;

public:
  reference
  operator()(S<T, A>& s, size_type i, size_type j)
  {
    return s.at(i, j);
  }

  const_reference
  operator()(S<T, A>& s, size_type i, size_type j) const
  {
    return s.at(i, j);
  }
};

template<typename T, typename A>
struct impl_at<utilz::square_matrix<T, A>, typename std::enable_if<utilz::traits::matrix_traits<T>::is_matrix::value>::type>
{
private:
  using size_type       = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using item_type       = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::item_type;
  using reference       = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::reference;
  using const_reference = typename utilz::traits::matrix_traits<utilz::square_matrix<T, A>>::const_reference;

private:
  reference
  at(utilz::square_matrix<T, A>& s, size_type i, size_type j)
  {
    impl_at<item_type>             get_at;
    impl_get_dimensions<item_type> get_dimensions;

    auto wf = false, hf = false;
    auto x = size_type(0), y = size_type(0);
    auto w = size_type(0), h = size_type(0);
    for (auto z = size_type(0); z < s.size() && (!wf || !hf); ++z) {
      auto dimensions = get_dimensions(s.at(z, z));
      if (!hf) {
        if (i < h + dimensions.h()) {
          y = z;
          hf = true;
        } else {
          h = h + dimensions.h();
        }
      }
      if (!wf) {
        if (j < w + dimensions.w()) {
          x = z;
          wf = true;
        } else {
          w = w + dimensions.w();
        }
      }
    }

    return get_at(s.at(y, x), i - h, j - w);
  }

public:
  reference
  operator()(utilz::square_matrix<T, A>& s, size_type i, size_type j)
  {
    return this->at(s, i, j);
  }

  const_reference
  operator()(utilz::square_matrix<T, A>& s, size_type i, size_type j) const
  {
    return this->at(s, i, j);
  }
};

template<typename S>
struct impl_replace
{
  static_assert(utilz::traits::matrix_traits<S>::is_matrix::value, "erro: input type has to be a matrix");

private:
  using size_type  = typename utilz::traits::matrix_traits<S>::size_type;
  using value_type = typename utilz::traits::matrix_traits<S>::value_type;

public:
  void
  operator()(S& s, value_type f, value_type t)
  {
    matrix_at<S>             get_at;
    matrix_get_dimensions<S> get_dimensions;

    auto dimensions = get_dimensions(s);
    for (auto i = size_type(0); i < dimensions.h(); ++i)
      for (auto j = size_type(0); j < dimensions.w(); ++j)
        if (get_at(s, i, j) == f)
          get_at(s, i, j) = t;
  }
};

template<typename S>
struct impl_set_dimensions
{
  static_assert(utilz::traits::matrix_traits<S>::is::value, "erro: input type has to be a matrix");
};

template<typename T, typename A>
struct impl_set_dimensions<utilz::square_matrix<T, A>>
{
private:
  using size_type = typename utilz::traits::matrix_traits<square_matrix<T, A>>::size_type;

public:
  void
  operator()(utilz::square_matrix<T, A>& s, size_type total_size)
  {
    s = utilz::square_matrix<T, A>(total_size, s.get_allocator());
  }
};

template<typename T, typename A, typename U>
struct impl_set_dimensions<utilz::square_matrix<utilz::square_matrix<T, A>, U>>
{
private:
  using size_type = typename utilz::traits::matrix_traits<utilz::square_matrix<utilz::square_matrix<T, A>, U>>::size_type;

public:
  void
  operator()(utilz::square_matrix<utilz::square_matrix<T, A>>& s, size_type total_size, size_type item_size)
  {
    auto own_size = total_size / item_size;
    if (total_size % item_size != size_type(0))
      ++own_size;

    s = utilz::square_matrix<utilz::square_matrix<T, A>>(own_size, s.get_allocator());
    for (auto i = size_type(0); i < s.size(); ++i)
      for (auto j = size_type(0); j < s.size(); ++j) {
        typename U::template rebind<A>::other allocator(s.get_allocator());

        s.at(i, j) = utilz::square_matrix<T, A>(item_size, allocator);
      }
  }
};

template<typename T, typename A, typename U>
struct impl_set_dimensions<utilz::square_matrix<utilz::rect_matrix<T, A>, U>>
{
private:
  using size_type = typename utilz::traits::matrix_traits<utilz::square_matrix<utilz::rect_matrix<T, A>, U>>::size_type;

public:
  void
  operator()(utilz::square_matrix<utilz::rect_matrix<T, A>>& s, size_type total_size, std::vector<size_type> item_sizes)
  {
    auto own_size = item_sizes.size();

    s = utilz::square_matrix<utilz::rect_matrix<T, A>>(own_size, s.get_allocator());
    for (auto i = size_type(0); i < s.size(); ++i)
      for (auto j = size_type(0); j < s.size(); ++j) {
        typename U::template rebind<A>::other allocator(s.get_allocator());

        s.at(i, i) = utilz::rect_matrix<T, A>(item_sizes[j], item_sizes[i], allocator);
      }
  }
};

} // namespace get_dimensions

} // namespace procedures
} // namespace utilz

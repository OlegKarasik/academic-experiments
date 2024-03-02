#pragma once

#include "rect-shape.hpp"
#include "square-shape.hpp"

namespace utilz {
namespace traits {

// ---
// Forward declarations
//

enum matrix_dimensions_variant
{
  matrix_dimensions_variant_rect   = 1,
  matrix_dimensions_variant_square = 2
};

template<matrix_dimensions_variant TVariant, typename S>
class matrix_dimensions
{
  static_assert(false, "The variant is not supported");
};

template<typename S>
struct rect_matrix_traits;

template<typename T, typename A>
struct rect_matrix_traits<rect_matrix<T, A>>;

template<typename S>
struct square_matrix_traits;

template<typename T, typename A>
struct square_matrix_traits<square_matrix<T, A>>;

template<typename S>
struct matrix_traits;

template<typename T, typename A>
struct matrix_traits<rect_matrix<T, A>>;

template<typename T, typename A>
struct matrix_traits<square_matrix<T, A>>;

namespace impl {

template<typename T>
struct matrix_dimensions;

} // namespace impl

//
// Forward declarations
// ---

template<typename S>
class matrix_dimensions<matrix_dimensions_variant::matrix_dimensions_variant_rect, S>
  : public impl::matrix_dimensions<typename utilz::traits::rect_matrix_traits<S>::size_type>
{
public:
  using size_type = typename utilz::traits::rect_matrix_traits<S>::size_type;

public:
  matrix_dimensions()
    : impl::matrix_dimensions<size_type>(size_type(0), size_type(0))
  {
  }

  matrix_dimensions(size_type size)
    : impl::matrix_dimensions<size_type>(size, size)
  {
  }

  matrix_dimensions(size_type width, size_type height)
    : impl::matrix_dimensions<size_type>(width, height)
  {
  }

  matrix_dimensions(const S& matrix)
    : impl::matrix_dimensions<size_type>(matrix.width(), matrix.height())
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

template<typename S>
class matrix_dimensions<matrix_dimensions_variant::matrix_dimensions_variant_square, S>
  : public impl::matrix_dimensions<typename utilz::traits::square_matrix_traits<S>::size_type>
{
public:
  using size_type = typename utilz::traits::square_matrix_traits<S>::size_type;

public:
  matrix_dimensions()
    : impl::matrix_dimensions<size_type>(size_type(0), size_type(0))
  {
  }

  matrix_dimensions(size_type size)
    : impl::matrix_dimensions<size_type>(size, size)
  {
  }

  matrix_dimensions(const S& matrix)
    : impl::matrix_dimensions<size_type>(matrix.size(), matrix.size())
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

  operator size_type() const
  {
    return this->s();
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

template<typename T>
struct rect_matrix_traits
{
public:
  using is = std::bool_constant<false>;
};

template<typename T, typename A>
struct rect_matrix_traits<rect_matrix<T, A>>
{
public:
  using is              = std::bool_constant<true>;
  using item_type       = typename rect_matrix<T, A>::value_type;
  using size_type       = typename rect_matrix<T, A>::size_type;
  using value_type      = typename rect_matrix<T, A>::value_type;
  using pointer         = typename rect_matrix<T, A>::pointer;
  using reference       = typename rect_matrix<T, A>::reference;
  using const_reference = typename rect_matrix<T, A>::const_reference;
  using dimension_type  = matrix_dimensions<matrix_dimensions_variant::matrix_dimensions_variant_rect, rect_matrix<T, A>>;
};

template<typename T, typename A, typename U>
struct rect_matrix_traits<rect_matrix<rect_matrix<T, A>, U>>
{
public:
  using is              = std::bool_constant<true>;
  using item_type       = typename rect_matrix<rect_matrix<T, A>, U>::value_type;
  using size_type       = typename rect_matrix<rect_matrix<T, A>, U>::size_type;
  using value_type      = typename rect_matrix_traits<rect_matrix<T, A>>::value_type;
  using pointer         = typename rect_matrix_traits<rect_matrix<T, A>>::pointer;
  using reference       = typename rect_matrix_traits<rect_matrix<T, A>>::reference;
  using const_reference = typename rect_matrix_traits<rect_matrix<T, A>>::const_reference;
  using dimension_type  = matrix_dimensions<matrix_dimensions_variant::matrix_dimensions_variant_rect, rect_matrix<rect_matrix<T, A>, U>>;
};

template<typename T>
struct square_matrix_traits
{
public:
  using is = std::bool_constant<false>;
};

template<typename T, typename A>
struct square_matrix_traits<square_matrix<T, A>>
{
public:
  using is              = std::bool_constant<true>;
  using item_type       = typename square_matrix<T, A>::value_type;
  using size_type       = typename square_matrix<T, A>::size_type;
  using value_type      = typename square_matrix<T, A>::value_type;
  using pointer         = typename square_matrix<T, A>::pointer;
  using reference       = typename square_matrix<T, A>::reference;
  using const_reference = typename square_matrix<T, A>::const_reference;
  using dimension_type  = matrix_dimensions<matrix_dimensions_variant::matrix_dimensions_variant_square, square_matrix<T, A>>;
};

template<typename T, typename A, typename U>
struct square_matrix_traits<square_matrix<square_matrix<T, A>, U>>
{
public:
  using is              = std::bool_constant<true>;
  using item_type       = typename square_matrix<square_matrix<T, A>, U>::value_type;
  using size_type       = typename square_matrix<square_matrix<T, A>, U>::size_type;
  using value_type      = typename square_matrix_traits<square_matrix<T, A>>::value_type;
  using pointer         = typename square_matrix_traits<square_matrix<T, A>>::pointer;
  using reference       = typename square_matrix_traits<square_matrix<T, A>>::reference;
  using const_reference = typename square_matrix_traits<square_matrix<T, A>>::const_reference;
  using dimension_type  = matrix_dimensions<matrix_dimensions_variant::matrix_dimensions_variant_square, square_matrix<square_matrix<T, A>, U>>;
};

template<typename T>
struct matrix_traits
{
public:
  using is = std::bool_constant<false>;
};

template<typename T, typename A>
struct matrix_traits<utilz::rect_matrix<T, A>>
{
public:
  using is              = typename utilz::traits::rect_matrix_traits<utilz::rect_matrix<T, A>>::is;
  using item_type       = typename utilz::traits::rect_matrix_traits<utilz::rect_matrix<T, A>>::item_type;
  using value_type      = typename utilz::traits::rect_matrix_traits<utilz::rect_matrix<T, A>>::value_type;
  using size_type       = typename utilz::traits::rect_matrix_traits<utilz::rect_matrix<T, A>>::size_type;
  using reference       = typename utilz::traits::rect_matrix_traits<utilz::rect_matrix<T, A>>::reference;
  using const_reference = typename utilz::traits::rect_matrix_traits<utilz::rect_matrix<T, A>>::const_reference;
  using dimension_type  = typename utilz::traits::rect_matrix_traits<utilz::rect_matrix<T, A>>::dimension_type;

  static size_type
  get_width(const utilz::rect_matrix<T, A>& s)
  {
    return s.width();
  }
  static size_type
  get_height(const utilz::rect_matrix<T, A>& s)
  {
    return s.height();
  }
};

template<typename T, typename A>
struct matrix_traits<utilz::square_matrix<T, A>>
{
public:
  using is              = typename utilz::traits::square_matrix_traits<utilz::square_matrix<T, A>>::is;
  using item_type       = typename utilz::traits::square_matrix_traits<utilz::square_matrix<T, A>>::item_type;
  using value_type      = typename utilz::traits::square_matrix_traits<utilz::square_matrix<T, A>>::value_type;
  using size_type       = typename utilz::traits::square_matrix_traits<utilz::square_matrix<T, A>>::size_type;
  using reference       = typename utilz::traits::square_matrix_traits<utilz::square_matrix<T, A>>::reference;
  using const_reference = typename utilz::traits::square_matrix_traits<utilz::square_matrix<T, A>>::const_reference;
  using dimension_type  = typename utilz::traits::square_matrix_traits<utilz::square_matrix<T, A>>::dimension_type;

  static size_type
  get_width(const utilz::square_matrix<T, A>& s)
  {
    return s.size();
  }
  static size_type
  get_height(const utilz::square_matrix<T, A>& s)
  {
    return s.size();
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

} // namespace impl

} // namespace traits
} // namespace utilz

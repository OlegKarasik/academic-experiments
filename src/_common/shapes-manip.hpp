#pragma once

#include "rect-shape.hpp"
#include "square-shape.hpp"

#include "shapes-traits.hpp"

namespace utilz {
namespace procedures {

// ---
// Forward declarations
//

namespace get_dimensions {

template<typename S>
struct impl_dimensions;

template<typename S>
struct impl_at;

template<typename S>
struct impl_replace;

} // namespace get_dimensions

namespace set_size {

template<std::size_t I, typename S>
struct impl;

} // namespace set_size

template<typename S>
using matrix_get_dimensions = get_dimensions::impl_dimensions<S>;

template<typename S>
using square_matrix_set_size = set_size::impl<std::size_t(0), S>;

template<typename S>
using matrix_at = get_dimensions::impl_at<S>;

template<typename S>
using matrix_replace = get_dimensions::impl_replace<S>;

//
// Forward declarations
// ---

namespace get_dimensions {

template<typename S>
struct impl_dimensions
{
  static_assert(utilz::traits::matrix_traits<S>::is::value, "erro: input type has to be a matrix");

private:
  using item_type = typename utilz::traits::matrix_traits<S>::item_type;

public:
  using dimension_type = typename utilz::traits::matrix_traits<S>::dimension_type;

public:
  dimension_type
  operator()(const S& s)
  {
    return dimension_type(s);
  }
};

template<template<typename> typename S, typename T, typename A>
struct impl_dimensions<S<utilz::square_matrix<T, A>>>
{
  static_assert(utilz::traits::matrix_traits<S<utilz::square_matrix<T, A>>>::is::value, "erro: input type has to be a matrix");

private:
  using item_type = typename utilz::traits::matrix_traits<S<utilz::square_matrix<T, A>>>::item_type;

public:
  using dimension_type = typename utilz::traits::matrix_traits<S<utilz::square_matrix<T, A>>>::dimension_type;

public:
  dimension_type
  operator()(const S<utilz::square_matrix<T, A>>& s)
  {
    impl_dimensions<item_type> get_dimensions;
    return s.empty()
           ? dimension_type(0)
           : dimension_type(s) * get_dimensions(s.at(0, 0));
  }
};

template<template<typename> typename S, typename T, typename A>
struct impl_dimensions<S<utilz::rect_matrix<T, A>>>
{
private:
  using item_type = typename utilz::traits::matrix_traits<S<utilz::rect_matrix<T, A>>>::item_type;
  using size_type = typename utilz::traits::matrix_traits<S<utilz::rect_matrix<T, A>>>::size_type;

public:
  using dimension_type = typename utilz::traits::matrix_traits<S<utilz::rect_matrix<T, A>>>::dimension_type;

private:
  impl_dimensions<item_type> m_get_dimensions;

public:
  dimension_type
  operator()(const S<utilz::rect_matrix<T, A>>& s)
  {
    if (s.empty())
      return dimension_type(0);

    auto w = typename impl_dimensions<item_type>::dimension_type(0);
    for (auto i = size_type(0); i < s.width(); ++i)
      w = w + this->m_get_dimensions(s.at(0, i));

    auto h = typename impl_dimensions<item_type>::dimension_type(0);
    for (auto i = size_type(0); i < s.height(); ++i)
      h = h + this->m_get_dimensions(s.at(i, 0));

    return dimension_type(w.w(), h.h());
  }
};

template<typename S>
struct impl_at
{
  static_assert(utilz::traits::matrix_traits<S>::is::value, "erro: input type has to be a matrix");

private:
  using size_type       = typename utilz::traits::matrix_traits<S>::size_type;
  using reference       = typename utilz::traits::matrix_traits<S>::reference;
  using const_reference = typename utilz::traits::matrix_traits<S>::const_reference;

public:
  reference
  operator()(S& s, size_type i, size_type j)
  {
    return s.at(i, j);
  }

  const_reference
  operator()(S& s, size_type i, size_type j) const
  {
    return s.at(i, j);
  }
};

template<template<typename> typename S, typename T, typename A>
struct impl_at<S<utilz::square_matrix<T, A>>>
{
  static_assert(utilz::traits::matrix_traits<S<utilz::square_matrix<T, A>>>::is::value, "erro: input type has to be a matrix");

private:
  using size_type       = typename utilz::traits::matrix_traits<S<utilz::square_matrix<T, A>>>::size_type;
  using item_type       = typename utilz::traits::matrix_traits<S<utilz::square_matrix<T, A>>>::item_type;
  using reference       = typename utilz::traits::matrix_traits<S<utilz::square_matrix<T, A>>>::reference;
  using const_reference = typename utilz::traits::matrix_traits<S<utilz::square_matrix<T, A>>>::const_reference;

private:
  reference
  at(S<utilz::square_matrix<T, A>>& s, size_type i, size_type j)
  {
    impl_at<item_type>         get_at;
    impl_dimensions<item_type> get_dimensions;

    auto dimensions = get_dimensions(s.at(0, 0));
    return get_at(s.at(i / dimensions.h(), j / dimensions.w()), i % dimensions.h(), j % dimensions.w());
  }

public:
  reference
  operator()(S<utilz::square_matrix<T, A>>& s, size_type i, size_type j)
  {
    return this->at(s, i, j);
  }

  const_reference
  operator()(S<utilz::square_matrix<T, A>>& s, size_type i, size_type j) const
  {
    return this->at(s, i, j);
  }
};

template<template<typename> typename S, typename T, typename A>
struct impl_at<S<utilz::rect_matrix<T, A>>>
{
  static_assert(utilz::traits::matrix_traits<S<utilz::rect_matrix<T, A>>>::is::value, "erro: input type has to be a matrix");

private:
  using item_type       = typename utilz::traits::matrix_traits<S<utilz::rect_matrix<T, A>>>::item_type;
  using size_type       = typename utilz::traits::matrix_traits<S<utilz::rect_matrix<T, A>>>::size_type;
  using reference       = typename utilz::traits::matrix_traits<S<utilz::rect_matrix<T, A>>>::reference;
  using const_reference = typename utilz::traits::matrix_traits<S<utilz::rect_matrix<T, A>>>::const_reference;
  using dimension_type  = typename utilz::traits::matrix_traits<S<utilz::rect_matrix<T, A>>>::dimension_type;

private:
  reference
  at(S<utilz::rect_matrix<T, A>>& s, size_type i, size_type j)
  {
    impl_at<item_type>         get_at;
    impl_dimensions<item_type> get_dimensions;

    auto y = size_type(0), x = size_type(0), h = size_type(0), w = size_type(0);

    auto s_dimensions = dimension_type(s);
    for (; y < s_dimensions.h(); ++y) {
      auto dimensions = get_dimensions(s.at(y, 0));
      auto height     = h + dimensions.h();
      if (height > i)
        break;

      h = height;
    }
    for (; x < s_dimensions.w(); ++x) {
      auto dimensions = get_dimensions(s.at(y, 0));
      auto width      = w + dimensions.w();
      if (width > j)
        break;

      w = width;
    }
    return get_at(s.at(y, x), i - h, j - w);
  }

public:
  reference
  operator()(S<utilz::rect_matrix<T, A>>& s, size_type i, size_type j)
  {
    return this->at(s, i, j);
  }

  const_reference
  operator()(S<utilz::rect_matrix<T, A>>& s, size_type i, size_type j) const
  {
    return this->at(s, i, j);
  }
};

template<typename S>
struct impl_replace
{
  static_assert(utilz::traits::matrix_traits<S>::is::value, "erro: input type has to be a matrix");

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

} // namespace get_dimensions

namespace set_size {

template<std::size_t I, typename T>
struct __value
{
public:
  T value;
};

template<std::size_t I, typename S>
struct impl
{
  static_assert(traits::square_matrix_traits<S>::is::value, "erro: input type has to be a square_matrix");
};

template<std::size_t I, typename T, typename A>
struct impl<I, square_matrix<T, A>>
{
public:
  using result_type = typename traits::square_matrix_traits<square_matrix<T, A>>::size_type;

public:
  void
  operator()(square_matrix<T, A>& s, result_type sz)
  {
    s = square_matrix<T, A>(sz, s.get_allocator());
  }
};

template<std::size_t I, typename T, typename A, typename U>
struct impl<I, square_matrix<square_matrix<T, A>, U>>
  : public __value<I, typename traits::square_matrix_traits<square_matrix<square_matrix<T, A>, U>>::size_type>
  , public impl<I + 1, square_matrix<T, A>>
{
public:
  using result_type = typename traits::square_matrix_traits<square_matrix<square_matrix<T, A>, U>>::size_type;

public:
  template<typename... TArgs>
  impl(result_type sz, TArgs... args)
    : impl<I + 1, square_matrix<T, A>>(args...)
  {
    this->__value<I, result_type>::value = sz;
  }

  void
  operator()(square_matrix<square_matrix<T, A>, U>& s, result_type sz)
  {
    result_type in_sz = this->__value<I, result_type>::value;
    result_type os_sz = sz / in_sz;

    if (sz % in_sz != result_type(0))
      ++os_sz;

    s = square_matrix<square_matrix<T, A>, U>(os_sz, s.get_allocator());

    for (result_type i = result_type(0); i < os_sz; ++i)
      for (result_type j = result_type(0); j < os_sz; ++j) {
        typename U::template rebind<A>::other in_a(s.get_allocator());

        auto in_s = square_matrix<T, A>(in_a);

        this->impl<I + 1, square_matrix<T, A>>::operator()(in_s, in_sz);

        s.at(i, j) = std::move(in_s);
      }
  }
};

} // namespace set_size

} // namespace procedures
} // namespace utilz

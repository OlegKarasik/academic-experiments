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
struct square_matrix_at;

template<typename S>
struct square_matrix_get;

template<typename S>
struct square_matrix_set;

template<typename S>
struct square_matrix_replace;

//
// Forward declarations
// ---

namespace get_dimensions {

template<typename S>
struct impl_dimensions
{
  static_assert(utilz::traits::matrix_traits<S>::is::value, "erro: input type has to be a matrix");

public:
  using item_type      = typename utilz::traits::matrix_traits<S>::item_type;
  using size_type      = typename utilz::traits::matrix_traits<S>::size_type;
  using dimension_type = typename utilz::traits::matrix_traits<S>::dimension_type;

public:
  dimension_type
  operator()(const S& s)
  {
    // if item type is a matrix we need to calculate the dimensions
    //
    if constexpr (utilz::traits::matrix_traits<item_type>::is::value) {
      dimension_type dimensions(s);
      if constexpr (utilz::traits::square_matrix_traits<S>::is::value) {
        typename impl_dimensions<item_type>::dimension_type v;
        for (auto i = size_type(0); i < dimensions; ++i) {
          impl_dimensions<item_type> get_dimensions;
          v = v + get_dimensions(s.at(i, i));
        }
        return dimension_type(v);
      }

      static_assert("erro: input type has to be a square matrix");
    }
    // Return the native dimensions
    //
    return dimension_type(s);
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

template<typename S>
struct square_matrix_at
{
  static_assert(traits::square_matrix_traits<S>::is::value, "erro: input type has to be a square_matrix");
};

template<typename T, typename A>
struct square_matrix_at<square_matrix<T, A>>
{
private:
  using size_type = typename traits::square_matrix_traits<square_matrix<T, A>>::size_type;

public:
  using result_type = typename traits::square_matrix_traits<square_matrix<T, A>>::value_type;

public:
  result_type&
  operator()(square_matrix<T, A>& s, size_type i, size_type j)
  {
    return s.at(i, j);
  }
  const result_type&
  operator()(const square_matrix<T, A>& s, size_type i, size_type j)
  {
    return s.at(i, j);
  }
};

template<typename T, typename A, typename U>
struct square_matrix_at<square_matrix<square_matrix<T, A>, U>>
{
private:
  using size_type = typename traits::square_matrix_traits<square_matrix<square_matrix<T, A>, U>>::size_type;

public:
  using result_type = typename traits::square_matrix_traits<square_matrix<square_matrix<T, A>, U>>::value_type;

private:
  matrix_get_dimensions<square_matrix<T, A>> m_size;
  square_matrix_at<square_matrix<T, A>>      m_at;

public:
  result_type&
  operator()(square_matrix<square_matrix<T, A>, U>& s, size_type i, size_type j)
  {
    size_type size = this->m_size(s.at(0, 0));

    return this->m_at(s.at(i / size, j / size), i % size, j % size);
  }
  const result_type&
  operator()(const square_matrix<square_matrix<T, A>, U>& s, size_type i, size_type j)
  {
    size_type size = this->m_size(s.at(0, 0));

    return this->m_at(s.at(i / size, j / size), i % size, j % size);
  }
};

template<typename S>
struct square_matrix_get
{
private:
  static_assert(traits::square_matrix_traits<S>::is::value, "erro: input type has to be a square_matrix");

private:
  using size_type = typename traits::square_matrix_traits<S>::size_type;

public:
  using result_type = typename traits::square_matrix_traits<S>::value_type;

private:
  square_matrix_at<S> m_at;

public:
  result_type
  operator()(const S& s, size_type i, size_type j)
  {
    return this->m_at(s, i, j);
  }
};

template<typename S>
struct square_matrix_set
{
private:
  static_assert(traits::square_matrix_traits<S>::is::value, "erro: input type has to be a square_matrix");

private:
  using size_type = typename traits::square_matrix_traits<S>::size_type;

public:
  using result_type = typename traits::square_matrix_traits<S>::value_type;

private:
  square_matrix_at<S> m_at;

public:
  void
  operator()(S& s, size_type i, size_type j, result_type v)
  {
    this->m_at(s, i, j) = v;
  }
};

template<typename S>
struct square_matrix_replace
{
private:
  static_assert(traits::square_matrix_traits<S>::is::value, "erro: input type has to be a square_matrix");

private:
  using size_type  = typename traits::square_matrix_traits<S>::size_type;
  using value_type = typename traits::square_matrix_traits<S>::value_type;

public:
  using result_type = typename traits::square_matrix_traits<S>::value_type;

private:
  matrix_get_dimensions<S> m_size;
  square_matrix_at<S>      m_at;

public:
  void
  operator()(S& s, value_type f, value_type t)
  {
    size_type size = this->m_size(s);

    for (size_type i = size_type(0); i < size; ++i)
      for (size_type j = size_type(0); j < size; ++j)
        if (this->m_at(s, i, j) == f)
          this->m_at(s, i, j) = t;
  }
};

} // namespace procedures
} // namespace utilz
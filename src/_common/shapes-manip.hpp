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
using square_matrix_at = get_dimensions::impl_at<S>;

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

private:
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
        typename utilz::traits::matrix_traits<item_type>::dimension_type v;
        for (auto i = size_type(0); i < dimensions; ++i) {
          impl_dimensions<item_type> get_dimensions;
          v = v + get_dimensions(s.at(i, i));
        }
        return dimension_type(v);
      }

      static_assert("erro: input type has to be a square matrix");
    } else {
      // Return the native dimensions
      //
      return dimension_type(s);
    }
  }
};

template<typename S>
struct impl_at
{
  static_assert(utilz::traits::matrix_traits<S>::is::value, "erro: input type has to be a matrix");

private:
  using item_type       = typename utilz::traits::matrix_traits<S>::item_type;
  using size_type       = typename utilz::traits::matrix_traits<S>::size_type;
  using value_type      = typename utilz::traits::matrix_traits<S>::value_type;
  using reference       = typename utilz::traits::matrix_traits<S>::reference;
  using const_reference = typename utilz::traits::matrix_traits<S>::const_reference;
  using dimension_type  = typename utilz::traits::matrix_traits<S>::dimension_type;

private:
  reference
  at(S& s, size_type i, size_type j)
  {
    if constexpr (utilz::traits::matrix_traits<item_type>::is::value) {
      dimension_type dimensions(s);
      if constexpr (utilz::traits::square_matrix_traits<S>::is::value) {
        // we are navigating across matrix diagonal to pin-point the approximate
        // location of the block
        //
        typename utilz::traits::matrix_traits<item_type>::dimension_type v;
        for (auto z = size_type(0), w = v.w(), h = v.h(); z < dimensions; ++z, w = v.w(), h = v.h()) {
          impl_dimensions<item_type> get_dimensions;
          v = v + get_dimensions(s.at(z, z));

          if (i < v.h() && j < v.w()) {
            // we have a hit on a diagonal
            //
            impl_at<item_type> get_at;
            return get_at(s.at(z, z), i - h, j - w);
          }

          if (i < v.h()) {
            // we have a hit on a row
            //
            for (auto x = z + size_type(1), w = v.w(); x < dimensions; ++x, w = v.w()) {
              v = v + get_dimensions(s.at(z, x));
              if (j < v.w()) {
                impl_at<item_type> get_at;
                return get_at(s.at(z, x), i - h, j - w);
              }
            }
            break;
          }
          if (j < v.w()) {
            // we have a hit on a column
            //
            for (auto y = z + size_type(1), h = v.h(); y < dimensions; ++y, h = v.h()) {
              v = v + get_dimensions(s.at(y, z));
              if (i < v.h()) {
                impl_at<item_type> get_at;
                return get_at(s.at(y, z), i - h, j - w);
              }
            }
            break;
          }
        }
        throw std::out_of_range("erro: indeces are out of range");
      }

      static_assert("erro: input type has to be a square matrix");
    } else {
      // Access matrix directly
      //
      return s.at(i, j);
    }
  }

public:
  reference
  operator()(S& s, size_type i, size_type j)
  {
    return this->at(s, i, j);
  }

  const_reference
  operator()(S& s, size_type i, size_type j) const
  {
    return this->at(s, i, j);
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
  operator()(S& s, size_type i, size_type j)
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

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
struct impl_width;

template<typename S>
struct impl_height;

} // namespace get_dimensions

namespace set_size {

template<std::size_t I, typename S>
struct impl;

} // namespace set_size

template<typename S>
using shape_get_width = get_dimensions::impl_width<S>;

template<typename S>
using shape_get_height = get_dimensions::impl_height<S>;

template<typename S>
using square_shape_set_size = set_size::impl<std::size_t(0), S>;

template<typename S>
struct square_shape_at;

template<typename S>
struct square_shape_get;

template<typename S>
struct square_shape_set;

template<typename S>
struct square_shape_replace;

//
// Forward declarations
// ---

namespace get_dimensions {

template<typename S>
struct impl_width
{
  static_assert(utilz::traits::shape_traits<S>::is::value, "erro: input type has to be a shape");

public:
  using item_type = typename utilz::traits::shape_traits<S>::item_type;
  using size_type = typename utilz::traits::shape_traits<S>::size_type;

public:
  size_type
  operator()(const S& s)
  {
    // if item type is a shape we need to calculate the width
    //
    if constexpr (utilz::traits::shape_traits<item_type>::is::value) {
      impl_width<item_type> size;

      auto result = size_type(0);
      for (auto i = size_type(0); i < utilz::traits::shape_traits<S>::get_width(s); ++i)
        result += size(s.at(0, i));

      return result;
    }
    // Return the native width if item type is not a shape
    //
    return utilz::traits::shape_traits<S>::get_width(s);
  }
};

template<typename S>
struct impl_height
{
  static_assert(utilz::traits::shape_traits<S>::is::value, "erro: input type has to be a square_shape");

public:
  using item_type = typename utilz::traits::shape_traits<S>::item_type;
  using size_type = typename utilz::traits::shape_traits<S>::size_type;

public:
  size_type
  operator()(const S& s)
  {
    // if item type is a shape we need to calculate the height
    //
    if constexpr (utilz::traits::shape_traits<item_type>::is::value) {
      impl_height<item_type> size;

      auto result = size_type(0);
      for (auto i = size_type(0); i < utilz::traits::shape_traits<S>::get_height(s); ++i)
        result += size(s.at(0, i));

      return result;
    }
    // Return the native height if item type is not a shape
    //
    return utilz::traits::shape_traits<S>::get_height(s);
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
  static_assert(traits::square_shape_traits<S>::is::value, "erro: input type has to be a square_shape");
};

template<std::size_t I, typename T, typename A>
struct impl<I, square_shape<T, A>>
{
public:
  using result_type = typename traits::square_shape_traits<square_shape<T, A>>::size_type;

public:
  void
  operator()(square_shape<T, A>& s, result_type sz)
  {
    s = square_shape<T, A>(sz, s.get_allocator());
  }
};

template<std::size_t I, typename T, typename A, typename U>
struct impl<I, square_shape<square_shape<T, A>, U>>
  : public __value<I, typename traits::square_shape_traits<square_shape<square_shape<T, A>, U>>::size_type>
  , public impl<I + 1, square_shape<T, A>>
{
public:
  using result_type = typename traits::square_shape_traits<square_shape<square_shape<T, A>, U>>::size_type;

public:
  template<typename... TArgs>
  impl(result_type sz, TArgs... args)
    : impl<I + 1, square_shape<T, A>>(args...)
  {
    this->__value<I, result_type>::value = sz;
  }

  void
  operator()(square_shape<square_shape<T, A>, U>& s, result_type sz)
  {
    result_type in_sz = this->__value<I, result_type>::value;
    result_type os_sz = sz / in_sz;

    if (sz % in_sz != result_type(0))
      ++os_sz;

    s = square_shape<square_shape<T, A>, U>(os_sz, s.get_allocator());

    for (result_type i = result_type(0); i < os_sz; ++i)
      for (result_type j = result_type(0); j < os_sz; ++j) {
        typename U::template rebind<A>::other in_a(s.get_allocator());

        auto in_s = square_shape<T, A>(in_a);

        this->impl<I + 1, square_shape<T, A>>::operator()(in_s, in_sz);

        s.at(i, j) = std::move(in_s);
      }
  }
};

} // namespace set_size

template<typename S>
struct square_shape_at
{
  static_assert(traits::square_shape_traits<S>::is::value, "erro: input type has to be a square_shape");
};

template<typename T, typename A>
struct square_shape_at<square_shape<T, A>>
{
private:
  using size_type = typename traits::square_shape_traits<square_shape<T, A>>::size_type;

public:
  using result_type = typename traits::square_shape_traits<square_shape<T, A>>::value_type;

public:
  result_type&
  operator()(square_shape<T, A>& s, size_type i, size_type j)
  {
    return s.at(i, j);
  }
  const result_type&
  operator()(const square_shape<T, A>& s, size_type i, size_type j)
  {
    return s.at(i, j);
  }
};

template<typename T, typename A, typename U>
struct square_shape_at<square_shape<square_shape<T, A>, U>>
{
private:
  using size_type = typename traits::square_shape_traits<square_shape<square_shape<T, A>, U>>::size_type;

public:
  using result_type = typename traits::square_shape_traits<square_shape<square_shape<T, A>, U>>::value_type;

private:
  shape_get_height<square_shape<T, A>> m_size;
  square_shape_at<square_shape<T, A>>  m_at;

public:
  result_type&
  operator()(square_shape<square_shape<T, A>, U>& s, size_type i, size_type j)
  {
    size_type size = this->m_size(s.at(0, 0));

    return this->m_at(s.at(i / size, j / size), i % size, j % size);
  }
  const result_type&
  operator()(const square_shape<square_shape<T, A>, U>& s, size_type i, size_type j)
  {
    size_type size = this->m_size(s.at(0, 0));

    return this->m_at(s.at(i / size, j / size), i % size, j % size);
  }
};

template<typename S>
struct square_shape_get
{
private:
  static_assert(traits::square_shape_traits<S>::is::value, "erro: input type has to be a square_shape");

private:
  using size_type = typename traits::square_shape_traits<S>::size_type;

public:
  using result_type = typename traits::square_shape_traits<S>::value_type;

private:
  square_shape_at<S> m_at;

public:
  result_type
  operator()(const S& s, size_type i, size_type j)
  {
    return this->m_at(s, i, j);
  }
};

template<typename S>
struct square_shape_set
{
private:
  static_assert(traits::square_shape_traits<S>::is::value, "erro: input type has to be a square_shape");

private:
  using size_type = typename traits::square_shape_traits<S>::size_type;

public:
  using result_type = typename traits::square_shape_traits<S>::value_type;

private:
  square_shape_at<S> m_at;

public:
  void
  operator()(S& s, size_type i, size_type j, result_type v)
  {
    this->m_at(s, i, j) = v;
  }
};

template<typename S>
struct square_shape_replace
{
private:
  static_assert(traits::square_shape_traits<S>::is::value, "erro: input type has to be a square_shape");

private:
  using size_type  = typename traits::square_shape_traits<S>::size_type;
  using value_type = typename traits::square_shape_traits<S>::value_type;

public:
  using result_type = typename traits::square_shape_traits<S>::value_type;

private:
  shape_get_height<S> m_size;
  square_shape_at<S>  m_at;

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

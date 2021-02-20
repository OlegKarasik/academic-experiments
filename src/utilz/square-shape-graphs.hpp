#pragma once

#include "graphs-generators.hpp"
#include "graphs-io.hpp"

#include "square-shape.hpp"

namespace utilz {
namespace graphs {

namespace ___set_size {

template<std::size_t I, typename T>
struct __value
{
private:
  template<typename __S>
  using _traits = traits::square_shape_traits<__S>;

  static_assert(_traits<T>::is::value, "erro: input type has to be a `square_shape<T>`");

private:
  template<typename __S>
  using _size_type = typename _traits<__S>::size_type;

public:
  _size_type<T> value;
};

template<std::size_t I, typename T>
struct __impl
{
};

template<std::size_t I, typename T, typename A>
struct __impl<I, square_shape<T, A>>
{
private:
  template<typename __S>
  using _traits = traits::square_shape_traits<__S>;

private:
  using type      = square_shape<T, A>;
  using size_type = typename _traits<type>::size_type;

public:
  void
  operator()(type& s, size_type sz)
  {
    s = type(sz);
  }
};

template<std::size_t I, typename T, typename A, typename U>
struct __impl<I, square_shape<square_shape<T, A>, U>>
  : public __value<I, square_shape<square_shape<T, A>, U>>
  , public __impl<I + 1, square_shape<T, A>>
{
private:
  template<typename __S>
  using _traits = traits::square_shape_traits<__S>;

private:
  using type      = square_shape<square_shape<T, A>, U>;
  using item_type = typename _traits<type>::item_type;
  using size_type = typename _traits<type>::size_type;

public:
  template<typename Q = size_type, typename... TArgs>
  __impl(Q sz, TArgs... args)
    : __impl<I + 1, item_type>(args...)
  {
    this->__value<I, type>::value = sz;
  }

  void
  operator()(type& s, size_type sz)
  {
    size_type in_sz = this->__value<I, type>::value;
    size_type os_sz = sz / in_sz;

    if (sz % in_sz != size_type(0))
      ++os_sz;

    s = type(os_sz);

    for (size_type i = size_type(0); i < os_sz; ++i)
      for (size_type j = size_type(0); j < os_sz; ++j)
        this->__impl<I + 1, item_type>::operator()(s.at(i, j), in_sz);
  }
};

} // namespace ___set_size

template<typename T>
using square_shape_set_size = ___set_size::__impl<std::size_t(0), T>;

namespace generators {

} // namespace generators

namespace io {

} // namespace io

} // namespace graphs
} // namespace utilz

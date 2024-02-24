#pragma once

#include "rect-shape.hpp"
#include "square-shape.hpp"

namespace utilz {
namespace traits {

// ---
// Forward declarations
//

template<typename S>
struct rect_shape_traits;

template<typename T, typename A>
struct rect_shape_traits<rect_shape<T, A>>;

template<typename S>
struct square_shape_traits;

template<typename T, typename A>
struct square_shape_traits<square_shape<T, A>>;

template<typename S>
struct shape_traits;

template<typename T, typename A>
struct shape_traits<rect_shape<T, A>>;

template<typename T, typename A>
struct shape_traits<square_shape<T, A>>;

//
// Forward declarations
// ---

template<typename T>
struct rect_shape_traits
{
public:
  using is = std::bool_constant<false>;
};

template<typename T, typename A>
struct rect_shape_traits<rect_shape<T, A>>
{
public:
  using is         = std::bool_constant<true>;
  using item_type  = typename rect_shape<T, A>::value_type;
  using size_type  = typename rect_shape<T, A>::size_type;
  using value_type = typename rect_shape<T, A>::value_type;
  using pointer    = typename rect_shape<T, A>::pointer;
};

template<typename T, typename A, typename U>
struct rect_shape_traits<rect_shape<rect_shape<T, A>, U>>
{
public:
  using is         = std::bool_constant<true>;
  using item_type  = typename rect_shape<rect_shape<T, A>, U>::value_type;
  using size_type  = typename rect_shape<rect_shape<T, A>, U>::size_type;
  using value_type = typename rect_shape_traits<rect_shape<T, A>>::value_type;
  using pointer    = typename rect_shape_traits<rect_shape<T, A>>::pointer;
  using reference  = typename rect_shape_traits<rect_shape<T, A>>::reference;
};

template<typename T>
struct square_shape_traits
{
public:
  using is = std::bool_constant<false>;
};

template<typename T, typename A>
struct square_shape_traits<square_shape<T, A>>
{
public:
  using is         = std::bool_constant<true>;
  using item_type  = typename square_shape<T, A>::value_type;
  using size_type  = typename square_shape<T, A>::size_type;
  using value_type = typename square_shape<T, A>::value_type;
  using pointer    = typename square_shape<T, A>::pointer;
  using reference  = typename square_shape<T, A>::reference;
};

template<typename T, typename A, typename U>
struct square_shape_traits<square_shape<square_shape<T, A>, U>>
{
public:
  using is         = std::bool_constant<true>;
  using item_type  = typename square_shape<square_shape<T, A>, U>::value_type;
  using size_type  = typename square_shape<square_shape<T, A>, U>::size_type;
  using value_type = typename square_shape_traits<square_shape<T, A>>::value_type;
  using pointer    = typename square_shape_traits<square_shape<T, A>>::pointer;
  using reference  = typename square_shape_traits<square_shape<T, A>>::reference;
};

template<typename T>
struct shape_traits
{
public:
  using is = std::bool_constant<false>;
};

template<typename T, typename A>
struct shape_traits<utilz::rect_shape<T, A>>
{
public:
  using is         = typename utilz::traits::rect_shape_traits<utilz::rect_shape<T, A>>::is;
  using item_type  = typename utilz::traits::rect_shape_traits<utilz::rect_shape<T, A>>::item_type;
  using value_type = typename utilz::traits::rect_shape_traits<utilz::rect_shape<T, A>>::value_type;
  using size_type  = typename utilz::traits::rect_shape_traits<utilz::rect_shape<T, A>>::size_type;

  static size_type
  get_width(const utilz::rect_shape<T, A>& s)
  {
    return s.width();
  }
  static size_type
  get_height(const utilz::rect_shape<T, A>& s)
  {
    return s.height();
  }
};

template<typename T, typename A>
struct shape_traits<utilz::square_shape<T, A>>
{
public:
  using is         = typename utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::is;
  using item_type  = typename utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::item_type;
  using value_type = typename utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::value_type;
  using size_type  = typename utilz::traits::square_shape_traits<utilz::square_shape<T, A>>::size_type;

  static size_type
  get_width(const utilz::square_shape<T, A>& s)
  {
    return s.size();
  }
  static size_type
  get_height(const utilz::square_shape<T, A>& s)
  {
    return s.size();
  }
};

} // namespace traits
} // namespace utilz

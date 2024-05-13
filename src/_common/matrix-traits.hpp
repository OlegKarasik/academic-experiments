#pragma once

#include "matrix.hpp"

namespace utilz {
namespace matrices {
namespace traits {

// ---
// Forward declarations
//

template<typename S, class Enable = void>
struct matrix_traits;

//
// Forward declarations
// ---

template<typename T, class Enable>
struct matrix_traits
{
public:
  using is_matrix = std::bool_constant<false>;
  using is_type   = std::bool_constant<true>;
};

template<typename T, typename A>
struct matrix_traits<utilz::matrices::rect_matrix<T, A>, typename std::enable_if<utilz::matrices::traits::matrix_traits<T>::is_type::value>::type>
{
public:
  using is_matrix       = std::bool_constant<true>;
  using is_type         = std::bool_constant<false>;
  using item_type       = typename utilz::matrices::rect_matrix<T, A>::value_type;
  using value_type      = typename utilz::matrices::rect_matrix<T, A>::value_type;
  using size_type       = typename utilz::matrices::rect_matrix<T, A>::size_type;
  using pointer         = typename utilz::matrices::rect_matrix<T, A>::pointer;
  using reference       = typename utilz::matrices::rect_matrix<T, A>::reference;
  using const_reference = typename utilz::matrices::rect_matrix<T, A>::const_reference;
};

template<typename T, typename A>
struct matrix_traits<utilz::matrices::rect_matrix<T, A>, typename std::enable_if<utilz::matrices::traits::matrix_traits<T>::is_matrix::value>::type>
{
public:
  using is_matrix       = std::bool_constant<true>;
  using is_type         = std::bool_constant<false>;
  using item_type       = typename utilz::matrices::rect_matrix<T, A>::value_type;
  using value_type      = typename matrix_traits<T>::value_type;
  using size_type       = typename matrix_traits<T>::size_type;
  using pointer         = typename matrix_traits<T>::pointer;
  using reference       = typename matrix_traits<T>::reference;
  using const_reference = typename matrix_traits<T>::const_reference;
};

template<typename T, typename A>
struct matrix_traits<utilz::matrices::square_matrix<T, A>, typename std::enable_if<utilz::matrices::traits::matrix_traits<T>::is_type::value>::type>
{
public:
  using is_matrix       = std::bool_constant<true>;
  using is_type         = std::bool_constant<false>;
  using value_type      = typename utilz::matrices::square_matrix<T, A>::value_type;
  using size_type       = typename utilz::matrices::square_matrix<T, A>::size_type;
  using pointer         = typename utilz::matrices::square_matrix<T, A>::pointer;
  using reference       = typename utilz::matrices::square_matrix<T, A>::reference;
  using const_reference = typename utilz::matrices::square_matrix<T, A>::const_reference;
  using item_type       = typename utilz::matrices::square_matrix<T, A>::value_type;
};

template<typename T, typename A>
struct matrix_traits<utilz::matrices::square_matrix<T, A>, typename std::enable_if<utilz::matrices::traits::matrix_traits<T>::is_matrix::value>::type>
{
public:
  using is_matrix       = std::bool_constant<true>;
  using is_type         = std::bool_constant<false>;
  using item_type       = typename utilz::matrices::square_matrix<T, A>::value_type;
  using value_type      = typename matrix_traits<T>::value_type;
  using size_type       = typename matrix_traits<T>::size_type;
  using pointer         = typename matrix_traits<T>::pointer;
  using reference       = typename matrix_traits<T>::reference;
  using const_reference = typename matrix_traits<T>::const_reference;
};

} // namespace traits
} // namespace matrices
} // namespace utilz

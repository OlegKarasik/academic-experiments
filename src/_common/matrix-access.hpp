#pragma once

#include "matrix.hpp"
#include "matrix-traits.hpp"

namespace utilz {
namespace matrices {
namespace access {

// ---
// Forward declarations
//

enum matrix_access_schema
{
  matrix_access_schema_none          = 0,
  matrix_access_schema_square        = 1,
  matrix_access_schema_square_blocks = 2,
  matrix_access_schema_rect_blocks   = 3,
};

template<typename S>
class matrix_dimensions;

template<matrix_access_schema TSchema, typename S>
class matrix_access
{
  static_assert(false, "The matrix access schema is not supported");
};

//
// Forward declarations
// ---

template<typename S>
class matrix_dimensions
{
  static_assert(::utilz::matrices::traits::matrix_traits<S>::is_matrix, "Non-matrix types aren't allowed");

private:
  using size_type = ::utilz::matrices::traits::matrix_traits<S>::size_type;

private:
  const size_type m_w;
  const size_type m_h;

public:
  matrix_dimensions(size_type w, size_type h)
    : m_w(w)
    , m_h(h)
  {
  }

  size_type
  w() noexcept
  {
    return this->m_w;
  }

  size_type
  h() noexcept
  {
    return this->m_h;
  }
};

template<typename T, typename A>
class matrix_access<matrix_access_schema::matrix_access_schema_square, square_matrix<T, A>>
{
  static_assert(::utilz::matrices::traits::matrix_traits<T>::is_type, "Matrix<Matrix> types aren't allowed");

private:
  using matrix_type                 = square_matrix<T, A>;
  using matrix_dimensions_type      = matrix_dimensions<matrix_type>;

  using size_type                   = ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;
  using value_type                  = ::utilz::matrices::traits::matrix_traits<matrix_type>::value_type;

  using value_reference              = value_type&;
  using matrix_reference            = matrix_type&;
  using matrix_dimensions_reference = matrix_dimensions_type&;

private:
  matrix_reference             m_matrix;

  const matrix_dimensions_type m_matrix_dimensions;

public:
  matrix_access(matrix_reference matrix)
    : m_matrix(matrix)
    , m_matrix_dimensions(matrix.size(), matrix.size())
  {
  }

public:
  matrix_dimensions_reference
  dimensions() noexcept
  {
    return this->m_matrix_dimensions;
  }

  value_reference
  at(size_type i, size_type j) noexcept
  {
    return this->m_matrix.at(i, j);
  }
};

template<typename T, typename A>
class matrix_access<matrix_access_schema::matrix_access_schema_square_blocks, square_matrix<T, A>>
{
  static_assert(::utilz::matrices::traits::matrix_traits<T>::is_type, "Matrix<Matrix> types aren't allowed");

private:
  using matrix_type                 = square_matrix<T, A>;
  using matrix_dimensions_type      = matrix_dimensions<matrix_type>;

  using size_type                   = ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;
  using value_type                  = ::utilz::matrices::traits::matrix_traits<matrix_type>::value_type;

  using value_reference             = value_type&;
  using matrix_reference            = matrix_type&;
  using matrix_dimensions_reference = matrix_dimensions_type&;

private:
  matrix_reference             m_matrix;

  const matrix_dimensions_type m_matrix_dimensions;

  const size_type              m_block_size;
  const size_type              m_block_square_size;
  const size_type              m_block_row_size;

public:
  matrix_access(matrix_reference matrix, size_type block_size)
    : m_matrix(matrix)
    , m_matrix_dimensions(matrix.size(), matrix.size())
    , m_block_size(block_size)
    , m_block_square_size(block_size * block_size)
    , m_block_row_size(size_type(0))
  {
    const size_type sz = matrix.size();
    if (sz > size_type(0))
      m_block_row_size = (sz / block_size) * m_block_square_size;
  }

public:
  matrix_dimensions_reference
  dimensions() noexcept
  {
    return this->m_matrix_dimensions;
  }

  value_reference
  at(size_type i, size_type j) noexcept
  {
    const auto i_point = (i / this->m_block_size) * this->m_block_row_size;
    const auto j_point = (j / this->m_block_size) * this->m_block_square_size + j % this->m_block_size;
    return this->m_matrix.at(i_point, j_point);
  }
};

}
}
}

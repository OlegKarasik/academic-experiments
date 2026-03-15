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
  matrix_access_schema_flat          = 1,
  matrix_access_schema_square_blocks = 2,
  matrix_access_schema_rect_blocks   = 3,
};

template<typename S, class Enable = void>
class matrix_params;

template<typename S>
class matrix_dimensions;

template<matrix_access_schema TSchema, typename S, class Enable = void>
class matrix_access;

template<typename S, class Enable>
class matrix_params
{
  static_assert(false, "The matrix type is not supported");
};

template<matrix_access_schema TSchema, typename S, class Enable>
class matrix_access
{
  static_assert(false, "The matrix access schema is not supported");
};

//
// Forward declarations
// ---

template<typename T, typename A>
class matrix_params
  <
    square_matrix<T, A>,
    typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type
  >
{
private:
  using matrix_type = square_matrix<T, A>;
  using size_type   = typename ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;

  using matrix_params_reference      = matrix_params&;
  using matrix_params_move_reference = matrix_params&&;

public:
  matrix_params()
  {
  }

  matrix_params(const matrix_params_reference o)
  {
  }

  matrix_params(matrix_params_move_reference o)
  {
  }

  matrix_params_reference
  operator=(const matrix_params_reference o)
  {
    return *this;
  };

  matrix_params_reference
  operator=(matrix_params_move_reference o) noexcept
  {
    return *this;
  };
};

template<typename T, typename A>
class matrix_params
  <
    rect_matrix<T, A>,
    typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type
  >
{
private:
  using matrix_type = rect_matrix<T, A>;
  using size_type   = typename ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;

  using matrix_params_reference      = matrix_params&;
  using matrix_params_move_reference = matrix_params&&;

public:
  matrix_params()
  {
  }

  matrix_params(const matrix_params_reference o)
  {
  }

  matrix_params(matrix_params_move_reference o)
  {
  }

  matrix_params_reference
  operator=(const matrix_params_reference o)
  {
    return *this;
  };

  matrix_params_reference
  operator=(matrix_params_move_reference o) noexcept
  {
    return *this;
  };
};

template<typename T, typename A, typename U>
class matrix_params
  <
    square_matrix<square_matrix<T, A>, U>,
    typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type
  >
{
private:
  using matrix_type = square_matrix<square_matrix<T, A>, U>;
  using size_type   = typename ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;

  using matrix_params_reference      = matrix_params&;
  using matrix_params_move_reference = matrix_params&&;

private:
  size_type m_block_size;

public:
  matrix_params()
    : m_block_size(size_type(0))
  {
  }
  matrix_params(size_type block_size)
    : m_block_size(block_size)
  {
  }

  matrix_params(const matrix_params_reference o)
    : m_block_size(o.m_block_size)
  {
  }

  matrix_params(matrix_params_move_reference o)
    : m_block_size(std::exchange(o.m_block_size, size_type(0)))
  {
  }

  size_type
  block_size() noexcept
  {
    return this->m_block_size;
  }

  matrix_params_reference
  operator=(const matrix_params_reference o)
  {
    if (this != &o) {
      this->m_block_size = o.m_block_size;
    }
    return *this;
  };

  matrix_params_reference
  operator=(matrix_params_move_reference o) noexcept
  {
    if (this != &o) {
      this->m_block_size = std::exchange(o.m_block_size, size_type(0));
    };

    return *this;
  };
};

template<typename T, typename A, typename U>
class matrix_params
  <
    square_matrix<rect_matrix<T, A>, U>,
    typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type
  >
{
private:
  using matrix_type = square_matrix<T, A>;
  using size_type   = typename ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;

  using matrix_params_reference      = matrix_params&;
  using matrix_params_move_reference = matrix_params&&;

private:
  std::vector<size_type> m_communities_sizes;

public:
  matrix_params()
    : m_communities_sizes(std::vector<size_type>())
  {
  }
  matrix_params(std::vector<size_type>& communities_sizes)
    : m_communities_sizes(communities_sizes)
  {
  }
  matrix_params(std::map<size_type, std::vector<size_type>>& communities)
  {
    for (auto [key, value] : communities)
      this->m_communities_sizes.push_back(value.size());
  }

  matrix_params(const matrix_params_reference o)
    : m_communities_sizes(o.m_communities_sizes)
  {
  }

  matrix_params(matrix_params_move_reference o)
    : m_communities_sizes(std::move(o.m_communities_sizes))
  {
  }

  std::vector<size_type>&
  communities_sizes()
  {
    return this->m_communities_sizes;
  }

  matrix_params_reference
  operator=(const matrix_params_reference o)
  {
    if (this != &o) {
      this->m_communities_sizes = o.m_communities_sizes;
    }
    return *this;
  };

  matrix_params_reference
  operator=(matrix_params_move_reference o) noexcept
  {
    if (this != &o) {
      this->m_communities_sizes = std::move(o.m_communities_sizes);
    };

    return *this;
  };
};

template<typename S>
class matrix_dimensions
{
  static_assert(::utilz::matrices::traits::matrix_traits<S>::is_matrix::value, "Non-matrix types aren't allowed");

private:
  using size_type = typename ::utilz::matrices::traits::matrix_traits<S>::size_type;

  using matrix_dimensions_reference      = matrix_dimensions&;
  using matrix_dimensions_move_reference = matrix_dimensions&&;

private:
  size_type m_w;
  size_type m_h;

public:
  matrix_dimensions()
    : m_w(size_type(0))
    , m_h(size_type(0))
  {
  }

  matrix_dimensions(size_type w, size_type h)
    : m_w(w)
    , m_h(h)
  {
  }

  matrix_dimensions(const matrix_dimensions_reference o)
    : m_w(o.m_w)
    , m_h(o.m_h)
  {
  }

  matrix_dimensions(matrix_dimensions_move_reference o)
    : m_w(std::exchange(o.m_w, size_type(0)))
    , m_h(std::exchange(o.m_h, size_type(0)))
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

  size_type
  min() noexcept
  {
    return std::min(this->m_w, this->m_h);
  }

  size_type
  max() noexcept
  {
    return std::max(this->m_w, this->m_h);
  }

  matrix_dimensions_reference
  operator=(const matrix_dimensions_reference o)
  {
    if (this != &o) {
      this->m_w = o.m_w;
      this->m_h = o.m_h;
    }
    return *this;
  };

  matrix_dimensions_reference
  operator=(matrix_dimensions_move_reference o) noexcept
  {
    if (this != &o) {
      this->m_w = std::exchange(o.m_w, size_type(0));
      this->m_h = std::exchange(o.m_h, size_type(0));
    };

    return *this;
  };
};

template<typename T, typename A>
class matrix_access
  <
    matrix_access_schema::matrix_access_schema_flat,
    square_matrix<T, A>,
    typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type
  >
{
public:
  using schema_value = std::integral_constant<matrix_access_schema, matrix_access_schema_flat>;

private:
  using matrix_type                  = square_matrix<T, A>;
  using matrix_dimensions_type       = matrix_dimensions<matrix_type>;
  using matrix_params_type           = matrix_params<matrix_type>;
  using size_type                    = ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;
  using value_type                   = ::utilz::matrices::traits::matrix_traits<matrix_type>::value_type;

  using value_reference              = value_type&;
  using matrix_reference             = matrix_type&;
  using matrix_dimensions_reference  = matrix_dimensions_type&;
  using matrix_params_reference      = matrix_params_type&;
  using matrix_access_reference      = matrix_access&;
  using matrix_access_move_reference = matrix_access&&;

private:
  matrix_reference        m_matrix;
  matrix_params_reference m_matrix_params;

  matrix_dimensions_type  m_matrix_dimensions;

public:
  matrix_access(matrix_reference matrix, matrix_params_reference matrix_params)
    : m_matrix(matrix)
    , m_matrix_params(matrix_params)
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

  void
  set_all(value_type v) noexcept
  {
    for (auto i = size_type(0); i < this->m_matrix_dimensions.h(); ++i)
      for (auto j = size_type(0); j < this->m_matrix_dimensions.w(); ++j)
        this->m_matrix.at(i, j) = v;
  }

  void
  set_diagonal(value_type v) noexcept
  {
    for (auto i = size_type(0); i < this->m_matrix_dimensions.min(); ++i)
      this->m_matrix.at(i, i) = v;
  }
};

template<typename T, typename A>
class matrix_access
  <
    matrix_access_schema::matrix_access_schema_flat,
    rect_matrix<T, A>,
    typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type
  >
{
public:
  using schema_value = std::integral_constant<matrix_access_schema, matrix_access_schema_flat>;

private:
  using matrix_type                  = rect_matrix<T, A>;
  using matrix_dimensions_type       = matrix_dimensions<matrix_type>;
  using matrix_params_type           = matrix_params<matrix_type>;
  using size_type                   = ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;
  using value_type                  = ::utilz::matrices::traits::matrix_traits<matrix_type>::value_type;

  using value_reference              = value_type&;
  using matrix_reference             = matrix_type&;
  using matrix_dimensions_reference  = matrix_dimensions_type&;
  using matrix_params_reference      = matrix_params_type&;
  using matrix_access_reference      = matrix_access&;
  using matrix_access_move_reference = matrix_access&&;

private:
  matrix_reference        m_matrix;
  matrix_params_reference m_matrix_params;

  matrix_dimensions_type  m_matrix_dimensions;

public:
  matrix_access(matrix_reference matrix, matrix_params_reference matrix_params)
    : m_matrix(matrix)
    , m_matrix_params(matrix_params)
    , m_matrix_dimensions(matrix.width(), matrix.height())
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

  void
  set_all(value_type v) noexcept
  {
    for (auto i = size_type(0); i < this->m_matrix_dimensions.h(); ++i)
      for (auto j = size_type(0); j < this->m_matrix_dimensions.w(); ++j)
        this->m_matrix.at(i, j) = v;
  }

  void
  set_diagonal(value_type v) noexcept
  {
    for (auto i = size_type(0); i < this->m_matrix_dimensions.min(); ++i)
      this->m_matrix.at(i, i) = v;
  }
};

template<typename T, typename A, typename U>
class matrix_access
  <
    matrix_access_schema::matrix_access_schema_flat,
    square_matrix<square_matrix<T, A>, U>,
    typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type
  >
{
public:
  using schema_value = std::integral_constant<matrix_access_schema, matrix_access_schema_flat>;

private:
  using matrix_block_type           = square_matrix<T, A>;
  using matrix_type                 = square_matrix<matrix_block_type, U>;
  using matrix_dimensions_type      = matrix_dimensions<matrix_type>;
  using matrix_params_type          = matrix_params<matrix_type>;
  using size_type                   = ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;
  using value_type                  = ::utilz::matrices::traits::matrix_traits<matrix_type>::value_type;

  using value_reference             = value_type&;
  using matrix_reference            = matrix_type&;
  using matrix_dimensions_reference = matrix_dimensions_type&;
  using matrix_params_reference     = matrix_params_type&;

private:
  matrix_reference        m_matrix;
  matrix_params_reference m_matrix_params;

  size_type               m_block_size;
  matrix_dimensions_type  m_matrix_dimensions;

public:
  matrix_access(matrix_reference matrix, matrix_params_reference matrix_params)
    : m_matrix(matrix)
    , m_matrix_params(matrix_params)
    , m_block_size(matrix_params.block_size())
    , m_matrix_dimensions(matrix.size() * this->m_block_size, matrix.size() * this->m_block_size)
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
    auto ib = i / this->m_block_size;
    auto jb = j / this->m_block_size;
    auto bi = i % this->m_block_size;
    auto bj = j % this->m_block_size;

    return this->m_matrix.at(ib, jb).at(bi, bj);
  }

  void
  set_all(value_type v) noexcept
  {
    matrix_params<matrix_block_type> params;
    for (auto i = size_type(0); i < this->m_matrix.size(); ++i)
      for (auto j = size_type(0); j < this->m_matrix.size(); ++j) {
        matrix_access<matrix_access_schema_flat, matrix_block_type> access(this->m_matrix.at(i, j), params);

        access.set_all(v);
      }
  }

  void
  set_diagonal(value_type v) noexcept
  {
    matrix_params<matrix_block_type> params;
    for (auto i = size_type(0); i < this->m_matrix.size(); ++i) {
      matrix_access<matrix_access_schema_flat, matrix_block_type> access(this->m_matrix.at(i, i), params);

      access.set_diagonal(v);
    }
  }
};

template<typename T, typename A, typename U>
class matrix_access
  <
    matrix_access_schema::matrix_access_schema_flat,
    square_matrix<rect_matrix<T, A>, U>,
    typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type
  >
{
public:
  using schema_value = std::integral_constant<matrix_access_schema, matrix_access_schema_flat>;

private:
  using matrix_block_type           = rect_matrix<T, A>;
  using matrix_type                 = square_matrix<matrix_block_type, U>;
  using matrix_dimensions_type      = matrix_dimensions<matrix_type>;
  using matrix_params_type          = matrix_params<matrix_type>;
  using size_type                   = ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;
  using value_type                  = ::utilz::matrices::traits::matrix_traits<matrix_type>::value_type;

  using value_reference             = value_type&;
  using matrix_reference            = matrix_type&;
  using matrix_dimensions_reference = matrix_dimensions_type&;
  using matrix_params_reference     = matrix_params_type&;

private:
  matrix_reference        m_matrix;
  matrix_params_reference m_matrix_params;

  matrix_dimensions_type  m_matrix_dimensions;

  std::vector<size_type>  m_icache;
  std::vector<size_type>  m_ocache;

public:
  matrix_access(matrix_reference matrix, matrix_params_reference matrix_params)
    : m_matrix(matrix)
    , m_matrix_params(matrix_params)
  {
    auto rsize = size_type(0), csize = size_type(0);
    for (auto i = size_type(0); i < this->m_matrix.size(); ++i) {
      auto& block = this->m_matrix.at(i, i);

      rsize += block.height();
      csize += block.width();
    }

    this->m_matrix_dimensions = matrix_dimensions_type(csize, rsize);

    this->m_icache.reserve(this->m_matrix_dimensions.max());
    this->m_ocache.reserve(this->m_matrix_dimensions.max());

    auto coffset = size_type(0), roffset = size_type(0);

    for (auto z = size_type(0); z < this->m_matrix.size(); ++z) {
      auto& block = this->m_matrix.at(z, z);

      for (auto i = size_type(0); i < block.height(); ++i) {
        this->m_icache.push_back(z);
        this->m_ocache.push_back(roffset);
      }

      roffset += block.height();
      coffset += block.width();
    }
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
    auto ib = this->m_icache[i];
    auto jb = this->m_icache[j];
    auto bi = i - this->m_ocache[i];
    auto bj = j - this->m_ocache[j];

    return this->m_matrix.at(ib, jb).at(bi, bj);
  }

  void
  set_all(value_type v) noexcept
  {
    matrix_params<matrix_block_type> params;
    for (auto i = size_type(0); i < this->m_matrix.size(); ++i)
      for (auto j = size_type(0); j < this->m_matrix.size(); ++j) {
        matrix_access<matrix_access_schema_flat, matrix_block_type> access(this->m_matrix.at(i, j), params);

        access.set_all(v);
      }
  }

  void
  set_diagonal(value_type v) noexcept
  {
    matrix_params<matrix_block_type> params;
    for (auto i = size_type(0); i < this->m_matrix.size(); ++i) {
      matrix_access<matrix_access_schema_flat, matrix_block_type> access(this->m_matrix.at(i, i), params);

      access.set_diagonal(v);
    }
  }
};

// TODO: Implement and test and link
//
template<typename T, typename A>
class matrix_access
  <
    matrix_access_schema::matrix_access_schema_square_blocks,
    square_matrix<T, A>,
    typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type
  >
{
private:
  using matrix_type                 = square_matrix<T, A>;
  using matrix_dimensions_type      = matrix_dimensions<matrix_type>;
  using matrix_params_type          = matrix_params<matrix_type>;

  using size_type                   = ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;
  using value_type                  = ::utilz::matrices::traits::matrix_traits<matrix_type>::value_type;

  using value_reference             = value_type&;
  using matrix_reference            = matrix_type&;
  using matrix_dimensions_reference = matrix_dimensions_type&;
  using matrix_params_reference     = matrix_params_type&;

private:
  matrix_reference             m_matrix;
  matrix_params_reference      m_matrix_params;

  const matrix_dimensions_type m_matrix_dimensions;

public:
  matrix_access(matrix_reference matrix, matrix_params_reference matrix_params)
    : m_matrix(matrix)
    , m_matrix_params(matrix_params)
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

}
}
}

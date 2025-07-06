#pragma once

#include "matrix.hpp"
#include "matrix-manip.hpp"
#include "matrix-traits.hpp"

namespace utilz {
namespace matrices {

// ---
// Forward declarations
//

template<typename S, class Enable = void>
struct matrix_wrap;

//
// Forward declarations
// ---

template<typename S, class Enable>
class matrix_wrap
{
  static_assert(false, "The applied matrix type isn't supported");
};

template<typename T, typename A>
class matrix_wrap<square_matrix<T, A>, typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type>
{
public:
  using matrix_type     = square_matrix<T, A>;
  using size_type       = typename traits::matrix_traits<matrix_type>::size_type;
  using value_type      = typename traits::matrix_traits<matrix_type>::value_type;
  using dimensions_type = typename procedures::matrix_dimensions<procedures::matrix_dimensions_variant_square, size_type>;

private:
  using reference        = typename traits::matrix_traits<matrix_type>::reference;
  using const_reference  = typename traits::matrix_traits<matrix_type>::const_reference;
  using matrix_reference = matrix_type&;

private:
  matrix_reference m_matrix;

public:
  matrix_wrap(matrix_reference matrix)
    : m_matrix(matrix)
  {
  }

  void
  rebind()
  {
  }

  matrix_reference
  matrix()
  {
    return this->m_matrix;
  }

  dimensions_type
  dimensions()
  {
    return dimensions_type(this->m_matrix.size());
  }

  reference
  at(size_type i, size_type j)
  {
    return this->m_matrix.at(i, j);
  }
  reference
  at(size_type i, size_type j) const
  {
    return this->m_matrix.at(i, j);
  }
};

template<typename T, typename A, typename U>
class matrix_wrap<square_matrix<square_matrix<T, A>, U>, typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type>
{
public:
  using matrix_type     = square_matrix<square_matrix<T, A>, U>;
  using size_type       = typename traits::matrix_traits<matrix_type>::size_type;
  using value_type      = typename traits::matrix_traits<matrix_type>::value_type;
  using dimensions_type = typename procedures::matrix_dimensions<procedures::matrix_dimensions_variant_square, size_type>;

private:
  using reference         = typename traits::matrix_traits<matrix_type>::reference;
  using const_reference   = typename traits::matrix_traits<matrix_type>::const_reference;
  using matrix_reference  = matrix_type&;
  using matrix_dimensions = dimensions_type;

private:
  matrix_reference  m_matrix;
  matrix_dimensions m_matrix_dimensions;

  std::vector<int> m_cache;

private:
  std::tuple<size_type, size_type>
  translate_index(size_type index)
  {
    auto entry = this->m_cache[index];

    return std::make_tuple(entry & 0xFFFF, entry >> 16);
  }

  reference
  translate_at(size_type i, size_type j)
  {
    size_type r, ro, c, co;

    std::tie(r, ro) = this->translate_index(i);
    std::tie(c, co) = this->translate_index(j);

    return this->m_matrix.at(r, c).at(i - ro, j - co);
  }

public:
  matrix_wrap(matrix_reference matrix)
    : m_matrix(matrix)
  {
    this->rebind();
  }

  void
  rebind()
  {
    auto s = size_type(0);
    for (auto z = size_type(0); z < this->m_matrix.size(); ++z)
      s += this->m_matrix.at(z, z).size();

    this->m_matrix_dimensions = matrix_dimensions(s);
    this->m_cache.clear();
    this->m_cache.reserve(s);

    auto delta = size_type(0);
    for (auto z = size_type(0); z < this->m_matrix.size(); ++z) {
      auto size = this->m_matrix.at(z, z).size();

      for (auto i = size_type(0); i < size; ++i)
        this->m_cache.push_back(int(delta << 16 | z));

      delta += size;
    }
  }

  matrix_reference
  matrix()
  {
    return this->m_matrix;
  }

  dimensions_type
  dimensions()
  {
    return this->m_matrix_dimensions;
  }

  reference
  at(size_type i, size_type j)
  {
    return this->translate_at(i, j);
  }
  reference
  at(size_type i, size_type j) const
  {
    return this->translate_at(i, j);
  }
};

template<typename T, typename A, typename U>
class matrix_wrap<square_matrix<rect_matrix<T, A>, U>, typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type>
{
public:
  using matrix_type     = square_matrix<rect_matrix<T, A>, U>;
  using size_type       = typename traits::matrix_traits<matrix_type>::size_type;
  using value_type      = typename traits::matrix_traits<matrix_type>::value_type;
  using dimensions_type = typename procedures::matrix_dimensions<procedures::matrix_dimensions_variant_square, size_type>;

private:
  using reference         = typename traits::matrix_traits<matrix_type>::reference;
  using const_reference   = typename traits::matrix_traits<matrix_type>::const_reference;
  using matrix_reference  = matrix_type&;
  using matrix_dimensions = dimensions_type;

private:
  matrix_reference  m_matrix;
  matrix_dimensions m_matrix_dimensions;

  std::vector<int> m_row_cache;
  std::vector<int> m_col_cache;

private:
  std::tuple<size_type, size_type>
  translate_index(std::vector<int>& cache, size_type index)
  {
    auto entry = cache[index];

    return std::make_tuple(entry & 0xFFFF, entry >> 16);
  }

  reference
  translate_at(size_type i, size_type j)
  {
    size_type r, ro, c, co;

    std::tie(r, ro) = this->translate_index(this->m_row_cache, i);
    std::tie(c, co) = this->translate_index(this->m_col_cache, j);

    return this->m_matrix.at(r, c).at(i - ro, j - co);
  }

public:
  matrix_wrap(matrix_type& matrix)
    : m_matrix(matrix)
  {
    this->rebind();
  }

  void
  rebind()
  {
    auto row_size = size_type(0), col_size = size_type(0);
    for (auto z = size_type(0); z < this->m_matrix.size(); ++z) {
      auto& block = this->m_matrix.at(z, z);

      row_size += block.height();
      col_size += block.width();
    }

    this->m_matrix_dimensions = matrix_dimensions(row_size);

    this->m_row_cache.clear();
    this->m_col_cache.clear();
    this->m_row_cache.reserve(row_size);
    this->m_col_cache.reserve(col_size);

    auto col       = size_type(0), row       = size_type(0);
    auto col_delta = size_type(0), row_delta = size_type(0);

    for (auto z = size_type(0); z < this->m_matrix.size(); ++z) {
      auto& block = this->m_matrix.at(z, z);

      for (auto i = size_type(0); i < block.height(); ++i, ++row)
        this->m_row_cache.push_back(int(row_delta << 16 | z));

      for (auto j = size_type(0); j < block.width(); ++j,  ++col)
        this->m_col_cache.push_back(int(col_delta << 16 | z));

      row_delta += block.height();
      col_delta += block.width();
    }
  }

  matrix_reference
  matrix()
  {
    return this->m_matrix;
  }

  dimensions_type
  dimensions()
  {
    return this->m_matrix_dimensions;
  }

  reference
  at(size_type i, size_type j)
  {
    return this->translate_at(i, j);
  }
  reference
  at(size_type i, size_type j) const
  {
    return this->translate_at(i, j);
  }
};

} // namespace matrices
} // namespace utilz

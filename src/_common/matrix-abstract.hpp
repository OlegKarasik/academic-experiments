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
class matrix_abstract;

namespace impl {

template<typename S>
class impl_matrix_abstract;

};

//
// Forward declarations
// ---

template<typename S, class Enable>
class matrix_abstract
{
  static_assert(false, "The applied matrix type isn't supported");
};

template<typename T, typename A>
class matrix_abstract<square_matrix<T, A>, typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type>
  : public impl::impl_matrix_abstract<square_matrix<T, A>>
{
public:
  using matrix_type = square_matrix<T, A>;
  using size_type   = typename traits::matrix_traits<matrix_type>::size_type;
  using value_type  = typename traits::matrix_traits<matrix_type>::value_type;

private:
  using reference        = typename traits::matrix_traits<matrix_type>::reference;
  using const_reference  = typename traits::matrix_traits<matrix_type>::const_reference;
  using matrix_reference = matrix_type&;

protected:
  reference
  translate_at(size_type i, size_type j) override
  {
    return this->m_matrix.at(i, j);
  }

public:
  matrix_abstract(matrix_reference matrix)
    : impl::impl_matrix_abstract<square_matrix<T, A>>(matrix)
  {
    this->rebind();
  }

  void
  rebind()
  {
  }

  size_type
  size() const
  {
    return this->m_matrix.size();
  }
};

template<typename T, typename A, typename U>
class matrix_abstract<square_matrix<square_matrix<T, A>, U>, typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type>
  : public impl::impl_matrix_abstract<square_matrix<square_matrix<T, A>, U>>
{
public:
  using matrix_type = square_matrix<square_matrix<T, A>, U>;
  using size_type   = typename traits::matrix_traits<matrix_type>::size_type;
  using value_type  = typename traits::matrix_traits<matrix_type>::value_type;

private:
  using reference        = typename traits::matrix_traits<matrix_type>::reference;
  using const_reference  = typename traits::matrix_traits<matrix_type>::const_reference;
  using matrix_reference = matrix_type&;

private:
  std::vector<int> m_cache;

  size_type m_s;

protected:
  reference
  translate_at(size_type i, size_type j) override
  {
    auto [r, ro] = this->translation_entry_unpack(this->m_cache[i]);
    auto [c, co] = this->translation_entry_unpack(this->m_cache[j]);

    return this->m_matrix.at(r, c).at(i - ro, j - co);
  }

public:
  matrix_abstract(matrix_reference matrix)
    : impl::impl_matrix_abstract<square_matrix<square_matrix<T, A>, U>>(matrix)
    , m_s(size_type(0))
  {
    this->rebind();
  }

  void
  rebind()
  {
    auto s = size_type(0);
    for (auto z = size_type(0); z < this->m_matrix.size(); ++z)
      s += this->m_matrix.at(z, z).size();

    this->m_s = s;

    this->m_cache.clear();
    this->m_cache.reserve(this->m_s);

    auto delta = size_type(0);
    for (auto z = size_type(0); z < this->m_matrix.size(); ++z) {
      auto size = this->m_matrix.at(z, z).size();

      for (auto i = size_type(0); i < size; ++i)
        this->m_cache.push_back(this->translation_entry_pack(z, delta));

      delta += size;
    }
  }

  size_type
  size() const
  {
    return this->m_s;
  }
};

template<typename T, typename A, typename U>
class matrix_abstract<square_matrix<rect_matrix<T, A>, U>, typename std::enable_if<traits::matrix_traits<T>::is_type::value>::type>
  : public impl::impl_matrix_abstract<square_matrix<rect_matrix<T, A>, U>>
{
public:
  using matrix_type = square_matrix<rect_matrix<T, A>, U>;
  using size_type   = typename traits::matrix_traits<matrix_type>::size_type;
  using value_type  = typename traits::matrix_traits<matrix_type>::value_type;

private:
  using reference        = typename traits::matrix_traits<matrix_type>::reference;
  using const_reference  = typename traits::matrix_traits<matrix_type>::const_reference;
  using matrix_reference = matrix_type&;

private:
  std::vector<int> m_row_cache;
  std::vector<int> m_col_cache;

  size_type m_s;

protected:
  reference
  translate_at(size_type i, size_type j) override
  {
    auto [r, ro] = this->translation_entry_unpack(this->m_row_cache[i]);
    auto [c, co] = this->translation_entry_unpack(this->m_col_cache[j]);

    return this->m_matrix.at(r, c).at(i - ro, j - co);
  }

public:
  matrix_abstract(matrix_reference matrix)
    : impl::impl_matrix_abstract<square_matrix<rect_matrix<T, A>, U>>(matrix)
    , m_s(size_type(0))
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

    if (row_size != col_size)
      std::logic_error("The abstract must represent a square matrix");

    this->m_s = col_size;

    this->m_row_cache.clear();
    this->m_col_cache.clear();
    this->m_row_cache.reserve(this->m_s);
    this->m_col_cache.reserve(this->m_s);

    auto col       = size_type(0), row       = size_type(0);
    auto col_delta = size_type(0), row_delta = size_type(0);

    for (auto z = size_type(0); z < this->m_matrix.size(); ++z) {
      auto& block = this->m_matrix.at(z, z);

      for (auto i = size_type(0); i < block.height(); ++i, ++row)
        this->m_row_cache.push_back(this->translation_entry_pack(z, row_delta));

      for (auto j = size_type(0); j < block.width(); ++j,  ++col)
        this->m_col_cache.push_back(this->translation_entry_pack(z, col_delta));

      row_delta += block.height();
      col_delta += block.width();
    }
  }

  size_type
  size() const
  {
    return this->m_s;
  }
};

namespace impl {

template<typename S>
class impl_matrix_abstract
{
private:
  using size_type = typename traits::matrix_traits<S>::size_type;

  using reference        = typename traits::matrix_traits<S>::reference;
  using const_reference  = typename traits::matrix_traits<S>::const_reference;
  using matrix_reference = S&;

protected:
  const matrix_reference m_matrix;

protected:
  std::tuple<size_type, size_type>
  translation_entry_unpack(int entry)
  {
    return std::make_tuple(entry & 0xFFFF, entry >> 16);
  }

  int
  translation_entry_pack(size_type index, size_type offset)
  {
    return int(offset << 16 | index);
  }

protected:
  virtual
  reference
  translate_at(size_type i, size_type j) = 0;

protected:
  impl_matrix_abstract(matrix_reference matrix)
    : m_matrix(matrix)
  {
  }

public:
  matrix_reference
  matrix()
  {
    return this->m_matrix;
  }

  reference
  at(size_type i, size_type j)
  {
    return this->translate_at(i, j);
  }
  const_reference
  at(size_type i, size_type j) const
  {
    return this->translate_at(i, j);
  }
};

} // namespace impl

} // namespace matrices
} // namespace utilz

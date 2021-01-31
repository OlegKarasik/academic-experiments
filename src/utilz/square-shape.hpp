#pragma once

#include <exception>

namespace utilz {

template<typename T>
class square_shape
{
private:
  size_t m_s;
  T*     m_m;

public:
  square_shape()
    : m_m(nullptr)
    , m_s(0)
  {}
  square_shape(T* m, size_t s)
    : m_m(m)
    , m_s(s)
  {}
  square_shape(const square_shape& o)
    : m_m(o.m_m)
    , m_s(o.m_s)
  {}

  size_t s() const noexcept
  {
    return this->m_s;
  }

  T* operator()(size_t i) noexcept
  {
    return &this->m_m[i * this->m_s];
  }
  const T* operator()(size_t i) const noexcept
  {
    return &this->m_m[i * this->m_s];
  }

  T& operator()(size_t i, size_t j)
  {
    return this->m_m[i * this->m_s + j];
  }
  const T& operator()(size_t i, size_t j) const
  {
    return this->m_m[i * this->m_s + j];
  }

  bool operator==(const square_shape& o) const noexcept
  {
    return this == &o || (this->m_m == o.m_m && this->m_s == o.m_s);
  };
  bool operator!=(const square_shape& o) const noexcept
  {
    return !(*this == o);
  };

  square_shape& operator=(const square_shape& o)
  {
    if (this != &o) {
      this->m_s = o.m_s;
      this->m_m = o.m_m;
    }
    return *this;
  }
};

template<typename T>
class square_shape_s
{
private:
  square_shape<T> m_s;

  void _throwif_exceeds_s(size_t i) const
  {
    if (this->m_s.s() <= i)
      throw std::out_of_range("The indeces are outside of square shape's size");
  }

public:
  square_shape_s()
  {}
  square_shape_s(square_shape<T> s)
    : m_s(s)
  {}
  square_shape_s(const square_shape_s& o)
    : m_s(o.m_s)
  {}

  size_t s() const noexcept
  {
    return this->m_s.s();
  }

  T* operator()(size_t i) noexcept
  {
    this->_throwif_exceeds_s(i);

    return this->m_s(i);
  }
  const T* operator()(size_t i) const noexcept
  {
    this->_throwif_exceeds_s(i);

    return this->m_s(i);
  }

  T& operator()(size_t i, size_t j)
  {
    this->_throwif_exceeds_s(i);
    this->_throwif_exceeds_s(j);

    return this->m_s(i, j);
  }
  const T& operator()(size_t i, size_t j) const
  {
    this->_throwif_exceeds_s(i);
    this->_throwif_exceeds_s(j);

    return this->m_s(i, j);
  }

  bool operator==(const square_shape_s& o) const noexcept
  {
    return this == &o || this->m_s == o.m_s;
  };
  bool operator!=(const square_shape_s& o) const noexcept
  {
    return !(*this == o);
  };

  square_shape_s& operator=(const square_shape_s& o)
  {
    if (this != &o) {
      this->m_s = o.m_s;
    }
    return *this;
  }
};

} // namespace utilz

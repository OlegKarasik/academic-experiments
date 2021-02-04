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

} // namespace utilz

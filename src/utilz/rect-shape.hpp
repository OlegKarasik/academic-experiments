#pragma once

namespace utilz {

template<typename T>
class rect_shape
{
private:
  size_t m_w;
  size_t m_h;
  T*     m_m;

public:
  rect_shape()
    : m_m(nullptr)
    , m_w(0)
    , m_h(0)
  {}
  rect_shape(T* data, size_t width, size_t height)
    : m_m(data)
    , m_w(width)
    , m_h(height)
  {}
  rect_shape(const rect_shape& o)
    : m_m(o.m_m)
    , m_w(o.m_w)
    , m_h(o.m_h)
  {}

  size_t w() const noexcept
  {
    return this->m_w;
  }
  size_t h() const noexcept
  {
    return this->m_h;
  }

  T* begin() noexcept
  {
    return this->m_m;
  }
  const T* begin() const noexcept
  {
    return this->m_m;
  }

  T* end() noexcept
  {
    return this->m_m + this->m_w * this->m_h;
  }
  const T* end() const noexcept
  {
    return this->m_m + this->m_w * this->m_h;
  }

  T* operator()(size_t i) noexcept
  {
    return &this->m_m[i * this->m_w];
  }
  const T* operator()(size_t i) const noexcept
  {
    return &this->m_m[i * this->m_w];
  }

  T& operator()(size_t i, size_t j)
  {
    return this->m_m[i * this->m_w + j];
  }
  const T& operator()(size_t i, size_t j) const
  {
    return this->m_m[i * this->m_w + j];
  }

  bool operator==(const rect_shape& o) const noexcept
  {
    return this == &o || (this->m_m == o.m_m && this->m_w == o.m_w && this->m_h == o.m_h);
  };
  bool operator!=(const rect_shape& o) const noexcept
  {
    return !(*this == o);
  };

  rect_shape& operator=(const rect_shape& o)
  {
    if (this != &o) {
      this->m_h = o.m_h;
      this->m_w = o.m_w;
      this->m_m = o.m_m;
    }
    return *this;
  }
};

template<typename T>
class rect_shape_s
{
private:
  rect_shape<T> m_s;

  void _throwif_exceeds_h(size_t i) const
  {
    if (this->m_s.h() <= i)
      throw std::out_of_range("The indeces are outside of rectangular shape's height");
  }
  void _throwif_exceeds_w(size_t j) const
  {
    if (this->m_s.w() <= j)
      throw std::out_of_range("The indeces are outside of rectangular shape's width");
  }

public:
  rect_shape_s()
  {}
  rect_shape_s(rect_shape<T> s)
    : m_s(s)
  {}
  rect_shape_s(const rect_shape_s& o)
    : m_s(o.m_s)
  {}

  size_t w() const noexcept
  {
    return this->m_s.w();
  }
  size_t h() const noexcept
  {
    return this->m_s.h();
  }

  T* operator()(size_t i) noexcept
  {
    this->_throwif_exceeds_h(i);

    return this->m_s(i);
  }
  const T* operator()(size_t i) const noexcept
  {
    this->_throwif_exceeds_h(i);

    return this->m_s(i);
  }

  T& operator()(size_t i, size_t j)
  {
    this->_throwif_exceeds_h(i);
    this->_throwif_exceeds_w(j);

    return this->m_s(i, j);
  }
  const T& operator()(size_t i, size_t j) const
  {
    this->_throwif_exceeds_h(i);
    this->_throwif_exceeds_w(j);

    return this->m_s(i, j);
  }

  bool operator==(const rect_shape_s& o) const noexcept
  {
    return this == &o || this->m_s == o.m_s;
  };
  bool operator!=(const rect_shape_s& o) const noexcept
  {
    return !(*this == o);
  };

  rect_shape_s& operator=(const rect_shape_s& o)
  {
    if (this != &o) {
      this->m_s = o.m_s;
    }
    return *this;
  }
};

} // namespace utilz

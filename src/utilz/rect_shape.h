#pragma once

namespace utilz {

template<typename T>
class rect_shape
{
private:
  size_t m_w;
  size_t m_h;

  T* m_mem;

public:
  rect_shape()
    : m_mem(nullptr)
    , m_w(0)
    , m_h(0)
  {}
  rect_shape(T* data, size_t width, size_t height)
    : m_mem(data)
    , m_w(width)
    , m_h(height)
  {}
  rect_shape(const rect_shape& o)
    : m_mem(o.m_mem)
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

  T* operator()(size_t i) noexcept
  {
    return &this->m_mem[i * this->m_w];
  }
  const T* operator()(size_t i) const noexcept
  {
    return &this->m_mem[i * this->m_w];
  }

  T& operator()(size_t i, size_t j)
  {
    return this->m_mem[i * this->m_w + j];
  }
  const T& operator()(size_t i, size_t j) const
  {
    return this->m_mem[i * this->m_w + j];
  }

  bool operator==(const rect_shape& o) const noexcept
  {
    return this == &o || (this->m_mem == o.m_mem && this->m_w == o.m_w && this->m_h == o.m_h);
  };
  bool operator!=(const rect_shape& o) const noexcept
  {
    return !(*this == o);
  };

  rect_shape& operator=(const rect_shape& o)
  {
    if (this != &o) {
      this->m_h   = o.m_h;
      this->m_w   = o.m_w;
      this->m_mem = o.m_mem;
    }
    return *this;
  }
};

template<typename T>
class rect_shape_s
{
private:
  rect_shape<T> m_shape;

  void _throwif_exceeds_h(size_t i) const
  {
    if (this->m_shape.h() <= i)
      throw std::out_of_range("The indeces are outside of rectangular shape's height");
  }
  void _throwif_exceeds_w(size_t j) const
  {
    if (this->m_shape.w() <= j)
      throw std::out_of_range("The indeces are outside of rectangular shape's width");
  }

public:
  rect_shape_s()
  {}
  rect_shape_s(rect_shape<T>& s)
    : m_shape(s)
  {}
  rect_shape_s(const rect_shape<T>& s)
    : m_shape(s)
  {}
  rect_shape_s(const rect_shape_s& o)
    : m_shape(o.m_shape)
  {}

  size_t w() const noexcept
  {
    return this->m_shape.w();
  }
  size_t h() const noexcept
  {
    return this->m_shape.h();
  }

  T* operator()(size_t i) noexcept
  {
    this->_throwif_exceeds_h(i);

    return this->m_shape(i);
  }
  const T* operator()(size_t i) const noexcept
  {
    this->_throwif_exceeds_h(i);

    return this->m_shape(i);
  }

  T& operator()(size_t i, size_t j)
  {
    this->_throwif_exceeds_h(i);
    this->_throwif_exceeds_w(j);

    return this->m_shape(i, j);
  }
  const T& operator()(size_t i, size_t j) const
  {
    this->_throwif_exceeds_h(i);
    this->_throwif_exceeds_w(j);

    return this->m_shape(i, j);
  }

  bool operator==(const rect_shape_s& o) const noexcept
  {
    return this == &o || this->m_shape == o.m_shape;
  };
  bool operator!=(const rect_shape_s& o) const noexcept
  {
    return !(*this == o);
  };

  rect_shape_s& operator=(const rect_shape_s& o)
  {
    if (this != &o) {
      this->m_shape = o.m_shape;
    }
    return *this;
  }
};

template<typename T>
bool
is_square_shape(const rect_shape<T>& s)
{
  return s.w() == s.h();
}

} // namespace utilz

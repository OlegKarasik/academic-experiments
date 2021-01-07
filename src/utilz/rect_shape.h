#pragma once

namespace utilz {

template<typename T>
class rect_shape
{
private:
  size_t m_w;
  size_t m_h;

  T* m_mem;

  void _throwif(size_t i, size_t j) const
  {
    if (this->m_h <= i || this->m_w <= j)
      throw std::out_of_range("The indeces are outside of rectangular shape");
  }

public:
  class iterator
  {
  private:
    T* m_mem;

  public:
    iterator(T* mem)
      : m_mem(mem)
    {}

    iterator& operator++() noexcept
    {
      ++this->m_mem;
      return *this;
    }
    iterator& operator++(int) noexcept
    {
      iterator p = this->m_mem;

      ++(*this);

      return *this;
    }
    iterator& operator--() noexcept
    {
      --this->m_mem;
      return *this;
    }
    iterator& operator--(int) noexcept
    {
      iterator p = this->m_mem;

      --(*this);

      return *this;
    }

    T& operator*() noexcept
    {
      return *this->m_mem;
    }
    const T& operator*() const noexcept
    {
      return *this->m_mem;
    }

    T& operator->() noexcept
    {
      return *this->m_mem;
    }
    const T& operator->() const noexcept
    {
      return *this->m_mem;
    }

    bool operator==(const iterator& o) const noexcept
    {
      return this == &o || this->m_mem == o.m_mem;
    };
    bool operator!=(const iterator& o) const noexcept
    {
      return !(*this == o);
    };
  };

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

  iterator begin() noexcept
  {
    return iterator(this->m_mem);
  }
  iterator end() noexcept
  {
    return iterator(this->m_mem + (this->m_w * this->m_h));
  }

  T* operator[](size_t i) noexcept
  {
    return &this->m_mem[i * this->m_w];
  }
  const T* operator[](size_t i) const noexcept
  {
    return &this->m_mem[i * this->m_w];
  }

  T& operator()(size_t i, size_t j)
  {
    this->_throwif(i, j);
    return this->m_mem[i * this->m_w + j];
  }
  const T& operator()(size_t i, size_t j) const
  {
    this->_throwif(i, j);
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
bool
is_square_shape(const rect_shape<T>& s)
{
  return s.w() == s.h();
}

} // namespace utilz

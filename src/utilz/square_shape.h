#pragma once

using namespace std;

namespace utilz {

template<typename T>
class square_shape
{
  using pointer    = T*;
  using reference  = T&;
  using value_type = T;

private:
  size_t m_s;

  pointer m_p;

public:
  class iterator
  {
  private:
    pointer m_p;

  public:
    iterator(pointer data)
      : m_p(data)
    {}

    iterator& operator++()
    {
      ++this->m_p;
      return *this;
    }
    iterator& operator++(int)
    {
      iterator p = this->m_p;

      ++(*this);

      return *this;
    }
    iterator& operator--()
    {
      --this->m_p;
      return *this;
    }
    iterator& operator--(int)
    {
      iterator p = this->m_p;

      --(*this);

      return *this;
    }

    reference operator*() { return *this->m_p; }
    reference operator->() { return *this->m_p; }

    iterator& operator=(pointer v)
    {
      this->m_p = v;
      return *this;
    }
    iterator& operator=(const iterator& o)
    {
      this->m_p = o.m_p;
      return *this;
    }

    reference       operator[](size_t idx) { return this->m_p[idx]; }
    const reference operator[](size_t idx) const { return this->m_p[idx]; }

    bool operator==(const iterator& o) const noexcept
    {
      return this == &o || this->m_p == o.m_p;
    };
    bool operator!=(const iterator& o) const noexcept { return !(*this == o); };
  };

  square_shape()
    : m_p(nullptr)
    , m_s(0)
  {}
  square_shape(pointer data, size_t size)
    : m_p(data)
    , m_s(size)
  {}
  square_shape(const square_shape& o)
    : m_p(o.m_p)
    , m_s(o.m_s)
  {}

  size_t s() { return this->m_s; }

  iterator begin() { return iterator(this->m_p); }
  iterator end() { return iterator(this->m_p + (this->m_s * this->m_s)); }

  pointer   operator[](size_t i) { return &this->m_p[i * this->m_s]; }
  reference operator()(size_t i, size_t j)
  {
    if (this->m_s <= i || this->m_s <= j)
      throw std::out_of_range("erro: shape isn\'t large enough to hold the data");

    return this->m_p[i * this->m_s + j];
  }

  bool operator==(const square_shape& o) const noexcept
  {
    return this == &o || (this->m_p == o.m_p && this->m_s == o.m_s && this->m_h == o.m_h);
  };
  bool operator!=(const square_shape& o) const noexcept
  {
    return !(*this == o);
  };

  square_shape& operator=(const square_shape& o)
  {
    if (this != &o) {
      this->m_s = o.m_s;
      this->m_p = o.m_p;
    }
    return *this;
  }
};

} // namespace utilz

#pragma once

#include "square-shape.hpp"

namespace utilz {

template<typename T, T V>
class square_shape_memory
{
private:
  const T m_v = V;

  T*              m_m;
  square_shape<T> m_s;

public:
  square_shape_memory()
    : m_m(nullptr)
  {}
  ~square_shape_memory()
  {
    if (this->m_m != nullptr)
      delete[] this->m_m;
  }

  square_shape_memory& init(size_t v, size_t e)
  {
    if (this->m_m != nullptr)
      throw std::runtime_error("erro: can't reuse `square_shape_memory<T>` object");

    // We intentionally ignore `e` (edge) count value here
    // because for square_shape<T> it has no value
    //
    this->m_m = new T[v * v];
    if (this->m_m == nullptr)
      throw std::runtime_error("erro: can't allocate memory");

    std::fill_n(this->m_m, v * v, this->m_v);

    this->m_s = square_shape<T>(this->m_m, v);

    return *this;
  }

  explicit operator square_shape<T>()
  {
    return this->m_s;
  }
};

template<typename T, typename M>
class square_shape_out
{
private:
  M&                m_m;
  square_shape_s<T> m_s;

public:
  square_shape_out(M& m)
    : m_m(m)
  {}

  void init(size_t v, size_t e)
  {
    this->m_s = square_shape_s<T>((square_shape<T>)this->m_m.init(v, e));
  }

  T& set(size_t i, size_t j)
  {
    return this->m_s(i, j);
  }
};

template<typename T, typename P>
class square_shape_in
{
private:
  const square_shape_s<T> m_s;
  const P&                m_p;

  size_t m_e;

public:
  class iterator_value
  {
  private:
    size_t m_f;
    size_t m_t;
    T      m_w;

  public:
    iterator_value()
      : m_f(0)
      , m_t(0)
      , m_w(){};

    iterator_value(size_t f, size_t t, const T& w)
      : m_f(f)
      , m_t(t)
      , m_w(w)
    {
    }

    size_t f() const noexcept
    {
      return this->m_f;
    }

    size_t t() const noexcept
    {
      return this->m_t;
    }

    const T& w() const noexcept
    {
      return this->m_w;
    }
  };

  class iterator
  {
  private:
    const square_shape_s<T> m_s;
    const P&                m_p;

    size_t m_i;
    size_t m_j;

    iterator_value m_v;

  private:
    void
    _next()
    {
      // Ensure we do nothing if end is reached
      //
      if (this->m_i == this->m_s.s() && this->m_j == this->m_s.s())
        return;

      while (this->m_i < this->m_s.s()) {
        while (this->m_j < this->m_s.s()) {
          const T& v = this->m_s(this->m_i, this->m_j);
          if (this->m_p(v)) {
            this->m_v = iterator_value(this->m_i, this->m_j, v);

            // Increment column counter to ensure we are moving forward
            // on subsequent calls
            ++this->m_j;
            return;
          }

          ++this->m_j;
        }

        // In case when row counter isn't set to last row index,
        // we set column counter to zero and scan next row
        //
        if (++this->m_i != this->m_s.s())
          this->m_j = 0;
      }
    }

  public:
    iterator(const square_shape_s<T> s, const P& p, size_t i, size_t j)
      : m_s(s)
      , m_p(p)
      , m_i(i)
      , m_j(j)
    {
      this->_next();
    }

    iterator& operator++() noexcept
    {
      this->_next();

      return *this;
    }

    iterator_value operator*() noexcept
    {
      return this->m_v;
    }
    const iterator_value operator*() const noexcept
    {
      return this->m_v;
    }

    iterator_value operator->() noexcept
    {
      return this->m_v;
    }
    const iterator_value operator->() const noexcept
    {
      return this->m_v;
    }

    bool operator==(const iterator& o) const noexcept
    {
      return this == &o || this->m_s == o.m_s && this->m_i == o.m_i && this->m_j == o.m_j;
    };
    bool operator!=(const iterator& o) const noexcept
    {
      return !(*this == o);
    };
  };

  square_shape_in(const square_shape_s<T>& s, const P& p)
    : m_s(square_shape_s<T>(s))
    , m_p(p)
  {
    this->m_e = 0;

    for (auto it = this->begin(); it != this->end(); ++it)
      ++this->m_e;
  }

  size_t v() const noexcept
  {
    return this->m_s.s();
  }
  size_t e() const noexcept
  {
    return this->m_e;
  }

  iterator begin()
  {
    return iterator(this->m_s, this->m_p, 0, 0);
  }

  iterator end()
  {
    return iterator(this->m_s, this->m_p, this->m_s.s(), this->m_s.s());
  }
};

}

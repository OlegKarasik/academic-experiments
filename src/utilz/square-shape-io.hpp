#pragma once

#include "square-shape.hpp"

namespace utilz {

template<typename T>
class square_shape_io_proxy
{
private:
  square_shape<T> m_s;

public:
  square_shape_io_proxy()
    : m_s()
  {}
  square_shape_io_proxy(square_shape<T> s)
    : m_s(s)
  {
  }

  size_t s() const noexcept
  {
    return this->m_s.s();
  }

  T& operator()(size_t i, size_t j)
  {
    return this->m_s(i, j);
  }
  const T& operator()(size_t i, size_t j) const
  {
    return this->m_s(i, j);
  }

  bool operator==(const square_shape_io_proxy& o) const noexcept
  {
    return this == &o || this->m_s == o.m_s;
  };
  bool operator!=(const square_shape_io_proxy& o) const noexcept
  {
    return !(*this == o);
  };
};

template<typename T>
class blocked_square_shape_io_proxy
{
private:
  size_t                        m_sz;
  square_shape<square_shape<T>> m_s;

public:
  blocked_square_shape_io_proxy()
    : m_s()
    , m_sz(0)
  {}
  blocked_square_shape_io_proxy(square_shape<square_shape<T>> s, size_t sz)
    : m_s(s)
    , m_sz(sz)
  {
    for (size_t i = 0; i < this->m_s.s(); ++i)
      for (size_t j = 0; j < this->m_s.s(); ++j)
        if (this->m_s(i, j).s() != sz)
          throw std::logic_error("erro: all blocks in blocked square shape has to be equal");
  }

  size_t s() const noexcept
  {
    return this->m_sz;
  }

  T& operator()(size_t i, size_t j)
  {
    return this->m_s(i / this->m_sz, j / this->m_sz)(i % this->m_sz, j % this->m_sz);
  }
  const T& operator()(size_t i, size_t j) const
  {
    return this->m_s(i / this->m_sz, j / this->m_sz)(i % this->m_sz, j % this->m_sz);
  }

  bool operator==(const blocked_square_shape_io_proxy& o) const noexcept
  {
    return this == &o || this->m_s == o.m_s;
  };
  bool operator!=(const blocked_square_shape_io_proxy& o) const noexcept
  {
    return !(*this == o);
  };
};

template<typename T, typename M, typename P>
class square_shape_graph_out
{
private:
  M& m_m;
  P  m_p;

public:
  square_shape_graph_out(M& m)
    : m_m(m)
    , m_p(P())
  {}

  void init(size_t v, size_t e)
  {
    this->m_p = (P)this->m_m.init(v, e);
  }

  T& set(size_t i, size_t j)
  {
    return this->m_p(i, j);
  }
};

template<typename T, typename C, typename P>
class square_shape_graph_in
{
private:
  const P  m_p;
  const C& m_c;

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
    const P  m_p;
    const C& m_c;

    size_t m_i;
    size_t m_j;

    iterator_value m_v;

  private:
    void
    _next()
    {
      // Ensure we do nothing if end is reached
      //
      if (this->m_i == this->m_p.s() && this->m_j == this->m_p.s())
        return;

      while (this->m_i < this->m_p.s()) {
        while (this->m_j < this->m_p.s()) {
          const T& v = this->m_p(this->m_i, this->m_j);
          if (this->m_c(v)) {
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
        if (++this->m_i != this->m_p.s())
          this->m_j = 0;
      }
    }

  public:
    iterator(const P p, const C& c, size_t i, size_t j)
      : m_p(p)
      , m_c(c)
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
      return this == &o || this->m_p == o.m_p && this->m_i == o.m_i && this->m_j == o.m_j;
    };
    bool operator!=(const iterator& o) const noexcept
    {
      return !(*this == o);
    };
  };

  square_shape_graph_in(const P p, const C& c)
    : m_p(p)
    , m_c(c)
  {
    this->m_e = 0;

    for (auto it = this->begin(); it != this->end(); ++it)
      ++this->m_e;
  }

  size_t v() const noexcept
  {
    return this->m_p.s();
  }
  size_t e() const noexcept
  {
    return this->m_e;
  }

  iterator begin()
  {
    return iterator(this->m_p, this->m_c, 0, 0);
  }

  iterator end()
  {
    return iterator(this->m_p, this->m_c, this->m_p.s(), this->m_p.s());
  }
};

// ---

template<typename T, T V>
class square_shape_memory
{
private:
  const T m_v = V;

  size_t m_s;
  T*     m_m;

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
      throw std::logic_error("erro: can't reuse `square_shape_memory<T>` object");

    // We intentionally ignore `e` (edge) count value here
    // because for square_shape<T> it has no value
    //
    this->m_s = v;
    this->m_m = new T[v * v];

    std::fill_n(this->m_m, v * v, this->m_v);

    return *this;
  }

  explicit operator square_shape<T>()
  {
    return square_shape<T>(this->m_m, this->m_s);
  }

  explicit operator square_shape_io_proxy<T>()
  {
    return square_shape_io_proxy<T>(square_shape<T>(this->m_m, this->m_s));
  }
};
}

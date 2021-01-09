#pragma once

#include "rect_shape.h"

namespace utilz {

template<typename T, T V>
class rect_shape_matrix_memory
{
private:
  const T m_v = V;

  T*            m_mem;
  rect_shape<T> m_shape;

public:
  rect_shape_matrix_memory()
    : m_mem(nullptr)
  {}
  ~rect_shape_matrix_memory()
  {
    if (this->m_mem != nullptr)
      delete[] this->m_mem;
  }

  void prep(size_t w, size_t h)
  {
    if (this->m_mem != nullptr)
      throw std::runtime_error("erro: can't reuse memory object");

    this->m_mem = new T[w * h];
    if (this->m_mem == nullptr) {
      throw std::runtime_error("erro: can't allocate memory");
    }

    std::fill_n(this->m_mem, w * h, this->m_v);

    this->m_shape = rect_shape<T>(this->m_mem, w, h);
  }

  rect_shape<T>& operator()() noexcept
  {
    return this->m_shape;
  }
};

template<typename T, typename M>
class rect_shape_matrix_output
{
private:
  M&              m_memory;
  rect_shape_s<T> m_shape;

public:
  rect_shape_matrix_output(M& memory)
    : m_memory(memory)
  {}

  void prep(size_t w, size_t h)
  {
    this->m_memory.prep(w, h);

    this->m_shape = rect_shape_s<T>(this->m_memory());
  }

  T& operator()(size_t i, size_t j)
  {
    return this->m_shape(i, j);
  }
};

template<typename T>
class rect_shape_matrix_input
{
private:
  const rect_shape_s<T> m_shape;

public:
  rect_shape_matrix_input(const rect_shape<T>& s)
    : m_shape(rect_shape_s<T>(s))
  {}

  size_t w() const noexcept
  {
    return this->m_shape.w();
  }
  size_t h() const noexcept
  {
    return this->m_shape.h();
  }

  const T& operator()(size_t i, size_t j) const
  {
    return this->m_shape(i, j);
  }
};

}

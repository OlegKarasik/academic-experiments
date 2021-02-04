#pragma once

#include "square-shape.hpp"

#include <memory>
#include <vector>

namespace utilz {

template<typename GraphT, typename WeightT, typename SizeOp, typename ElementOp, typename ElementPredicate>
class square_shape_graph_edges_transform
{
private:
  SizeOp&           m_s;
  ElementOp&        m_e;
  ElementPredicate& m_p;

public:
  square_shape_graph_edges_transform(SizeOp& s, ElementOp& e, ElementPredicate& p)
    : m_s(s)
    , m_p(p)
    , m_e(e)
  {
  }

  bool
  operator()(const GraphT& g, size_t& f, size_t& t, WeightT& w, bool prev)
  {
    size_t v = this->m_s(g);

    // Ensure we do nothing if we reach the end of the graph
    //
    if (f >= v || t >= v)
      return false;

    WeightT _w = w;
    size_t  _f = f, _t = t;

    // In case of continues transformation we increment
    // `t` to ensure we don't stuck on successful match
    //
    if (prev)
      ++_t;

    while (_f < v) {
      while (_t < v) {
        _w = this->m_e(g, _f, _t);
        if (this->m_p(_w)) {
          f = _f;
          t = _t;
          w = _w;

          return true;
        }

        ++_t;
      }

      // In case when row counter isn't set to last row index,
      // we set column counter to zero and scan next row
      //
      if (++_f != v)
        _t = 0;
    }

    // There is no edges left in a graph
    //
    return false;
  }
};

template<typename GraphT, typename WeightT, typename SizeOp, typename EdgeTransform>
class square_shape_graph_count_op
{
private:
  SizeOp&        m_s;
  EdgeTransform& m_e;

public:
  square_shape_graph_count_op(SizeOp& s, EdgeTransform& e)
    : m_s(s)
    , m_e(e)
  {
  }

  void
  operator()(const GraphT& g, size_t& v, size_t& e)
  {
    // Initialize vertex count to shape size and edges count to zero
    //
    v = this->m_s(g);
    e = 0;

    EdgeTransform _e = this->m_e;

    WeightT w;
    bool    r = false;
    size_t  f = 0, t = 0;

    r = _e(g, f, t, w, r);
    while (r) {
      ++e;

      r = _e(g, f, t, w, r);
    }
  }
};

template<typename T, T Fill>
class square_shape_resize_graph_op
{
private:
  std::vector<std::shared_ptr<T>> m_m;

public:
  void operator()(square_shape<T>& s, size_t v, size_t e)
  {
    // Allocate memory ignoring `e` (edge) count
    //
    std::shared_ptr<T> p(new T[v * v]);

    // Fill memory with default value i.e. make default
    // shape initialization
    //
    std::fill_n(p.get(), v * v, Fill);

    // Save memory to vector
    //
    this->m_m.push_back(p);

    // Output shape
    //
    s = square_shape<T>(p.get(), v);
  }
};

template<typename T, T Fill>
class square_shape_of_shapes_resize_graph_op
{
private:
  std::vector<std::shared_ptr<T>>               m_m;
  std::vector<std::shared_ptr<square_shape<T>>> m_s;

  size_t m_sz;

public:
  square_shape_of_shapes_resize_graph_op(size_t sz)
    : m_sz(sz)
  {
  }

  void operator()(square_shape<square_shape<T>>& s, size_t v, size_t e)
  {
    // Precondition: v % this->m_sz = 0 && v > this->m_sz

    // Calculate inner shapes count
    //
    size_t c = v / this->m_sz;

    // Allocate memory for data and for ignoring `e` (edge) count
    //
    std::shared_ptr<T>               p_memory(new T[v * v]);
    std::shared_ptr<square_shape<T>> p_shapes(new square_shape<T>[c * c]);

    // Fill memory with default value i.e. make default
    // shape initialization
    //
    std::fill_n(p_memory.get(), v * v, Fill);

    // Save memory to vector
    //
    this->m_m.push_back(p_memory);
    this->m_s.push_back(p_shapes);

    // Layout memory to shapes
    //
    T*               data   = p_memory.get();
    square_shape<T>* shapes = p_shapes.get();

    for (size_t i = 0; i < c * c; ++i, data += this->m_sz * this->m_sz)
      shapes[i] = square_shape<T>(data, this->m_sz);

    // Output shape
    //
    s = square_shape<square_shape<T>>(shapes, c);
  }
};

template<typename T>
class square_shape_size_op
{
public:
  size_t operator()(const square_shape<T>& s)
  {
    return s.s();
  }
};

template<typename T>
class square_shape_of_shapes_size_op
{
  size_t m_sz;

public:
  square_shape_of_shapes_size_op(size_t sz)
    : m_sz(sz)
  {
  }

  size_t operator()(const square_shape<square_shape<T>>& s)
  {
    return s.s() * this->m_sz;
  }
};

template<typename T>
class square_shape_element_op
{
public:
  T& operator()(square_shape<T>& s, size_t i, size_t j)
  {
    return s(i, j);
  }
};

template<typename T>
class const_square_shape_element_op
{
public:
  const T& operator()(const square_shape<T>& s, size_t i, size_t j)
  {
    return s(i, j);
  }
};

template<typename T>
class square_shape_of_shapes_element_op
{
private:
  size_t m_sz;

public:
  square_shape_of_shapes_element_op(size_t sz)
    : m_sz(sz)
  {}

  T& operator()(square_shape<square_shape<T>>& s, size_t i, size_t j)
  {
    return s(i / this->m_sz, j / this->m_sz)(i % this->m_sz, j % this->m_sz);
  }
};

template<typename T>
class const_square_shape_of_shapes_element_op
{
private:
  size_t m_sz;

public:
  const_square_shape_of_shapes_element_op(size_t sz)
    : m_sz(sz)
  {}

  const T& operator()(const square_shape<square_shape<T>>& s, size_t i, size_t j)
  {
    return s(i / this->m_sz, j / this->m_sz)(i % this->m_sz, j % this->m_sz);
  }
};

}

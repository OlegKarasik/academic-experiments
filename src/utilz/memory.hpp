#pragma once

// global includes
//
#include <memory>
#include <stdexcept>

namespace utilz {
namespace memory {

class buff_buf
{
public:
  using value_type      = char;
  using pointer         = value_type*;
  using const_pointer   = const value_type*;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

private:
  size_type m_size;
  pointer   m_memory;

public:
  buff_buf()
    : m_memory(nullptr)
    , m_size(size_type())
  {
  }
  buff_buf(pointer memory, size_type size)
    : m_memory(memory)
    , m_size(size)
  {
  }

  inline pointer
  allocate(size_type size)
  {
    if (size > this->m_size)
      throw std::runtime_error("erro: not enough memory in buffer");

    pointer p = this->m_memory;

    this->m_size   = this->m_size - size;
    this->m_memory = this->m_memory + size;

    return p;
  };
  inline void
    deallocate(pointer, size_type){
      // There is no need for complex rent-return semantic, all we need is
      // a C++ way to allocate memory from previously reserved buffer
      //
    };

  inline bool
  operator==(buff_buf const& a)
  {
    return *this == a || this->m_memory == a.m_memory && this->m_size == a.m_size;
  };
  inline bool
  operator!=(buff_buf const& a)
  {
    return !operator==(a);
  };
};

template<typename T>
class buff_allocator
{
public:
  using value_type      = T;
  using pointer         = value_type*;
  using const_pointer   = const value_type*;
  using reference       = value_type&;
  using const_reference = const value_type&;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

private:
  buff_buf m_a;

public:
  template<typename U>
  struct rebind
  {
    using other = buff_allocator<U>;
  };

public:
  buff_allocator()
    : m_a(buff_buf())
  {
  }
  buff_allocator(const buff_buf& a)
    : m_a(a)
  {
  }

  template<typename U>
  inline buff_allocator(buff_allocator<U>& o)
    : m_a(o.m_a)
  {
  }

  inline pointer
  address(reference ref)
  {
    return &ref;
  };
  inline const_pointer
  address(const_reference const_ref)
  {
    return &const_ref;
  };

  inline pointer
  allocate(size_type size)
  {
    return static_cast<pointer>(this->m_a.allocate(size * sizeof(value_type)));
  };
  inline void
  deallocate(pointer p, size_type sz)
  {
    this->m_a.deallocate(p, sz);
  };

  inline size_type
  max_size() const
  {
    return (std::numeric_limits<size_type>::max)() / sizeof(value_type);
  };

  inline void
  construct(pointer p, const_reference t)
  {
    new (p) value_type(t);
  };
  inline void
  destroy(pointer p)
  {
    p->~value_type();
  };

  inline bool
  operator==(buff_allocator const& a)
  {
    return this == a || this->m_a == a.m_a;
  };
  inline bool
  operator!=(buff_allocator const& a)
  {
    return !operator==(a);
  };
};

} // namespace memory
} // namespace apsp

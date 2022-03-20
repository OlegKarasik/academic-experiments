#pragma once

// global includes
//
#include <memory>
#include <algorithm>
#include <limits>
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
  size_type                   m_alignment;
  size_type                   m_size;
  std::shared_ptr<value_type> m_mem;

  pointer m_cmem;

public:
  buff_buf()
    : m_mem(nullptr)
    , m_cmem(nullptr)
    , m_size(size_type())
    , m_alignment(size_type())
  {
  }
  buff_buf(std::shared_ptr<value_type> memory, size_type size, size_type alignment)
    : m_mem(memory)
    , m_cmem(memory.get())
    , m_size(size)
    , m_alignment(alignment)
  {
    if (alignment % 2 != 0)
      throw std::invalid_argument("erro: the alignment value has to be a power of 2");
  }

  inline pointer
  allocate(size_type size)
  {
    void* p;
    if (this->m_alignment != 0) {
      p = reinterpret_cast<void*>(this->m_cmem);
      if (!std::align(this->m_alignment, size, p, this->m_size))
        throw std::runtime_error("erro: not enough memory in buffer");

      // Update current to aligned value, which is going to be returned
      //
      this->m_cmem = reinterpret_cast<pointer>(p);
    } else {
      p = this->m_cmem;
    }

    if (size > this->m_size)
      throw std::runtime_error("erro: not enough memory in buffer");

    this->m_size = this->m_size - size;
    this->m_cmem = this->m_cmem + size;

    return reinterpret_cast<pointer>(p);
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
    return this == &a || this->m_mem == a.m_mem && this->m_size == a.m_size;
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
  template<typename U>
  friend class buff_allocator;

public:
  using value_type      = T;
  using pointer         = value_type*;
  using const_pointer   = const value_type*;
  using reference       = value_type&;
  using const_reference = const value_type&;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

  using propagate_on_container_move_assignment = std::true_type;

private:
  buff_buf* m_buff;

public:
  template<typename U>
  struct rebind
  {
    typedef buff_allocator<U> other;
  };

public:
  buff_allocator()
    : m_buff(nullptr)
  {}
  buff_allocator(buff_buf* buff)
    : m_buff(buff)
  {}
  buff_allocator(const buff_allocator& o)
    : m_buff(o.m_buff)
  {}
  buff_allocator(buff_allocator&& o)
    : m_buff(std::exchange(o.m_buff, nullptr))
  {}

  template<typename U>
  buff_allocator(buff_allocator<U>& o)
    : m_buff(o.m_buff)
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
    if (this->m_buff == nullptr)
      throw std::logic_error("erro: the allocator wasn't initialized with memory buffer");

    return reinterpret_cast<pointer>(this->m_buff->allocate(size * sizeof(value_type)));
  };
  inline void
  deallocate(pointer p, size_type sz)
  {
    if (this->m_buff == nullptr)
      throw std::logic_error("erro: the allocator wasn't initialized with memory buffer");

    this->m_buff->deallocate(reinterpret_cast<char*>(p), sz);
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
    return this == &a || this->m_buff == a.m_buff;
  };
  inline bool
  operator!=(buff_allocator const& a)
  {
    return !operator==(a);
  };
  buff_allocator&
  operator=(buff_allocator&& o)
  {
    if (this != &o) {
      this->m_buff = std::exchange(o.m_buff, nullptr);
    };

    return *this;
  };
};

} // namespace memory
} // namespace apsp

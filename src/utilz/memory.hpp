#pragma once

// global includes
//
#include <algorithm>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace utilz {
namespace memory {

class buffer
{
public:
  using value_type      = char;
  using pointer         = value_type*;
  using const_pointer   = const value_type*;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

public:
  virtual inline pointer
  allocate(size_type size) = 0;

  virtual inline void
    deallocate(pointer, size_type) = 0;
};

template<typename T>
class buffer_allocator
{
  template<typename U>
  friend class buffer_allocator;

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
  buffer* m_buffer;

public:
  template<typename U>
  struct rebind
  {
    typedef buffer_allocator<U> other;
  };

public:
  buffer_allocator()
    : m_buffer(nullptr)
  {}
  buffer_allocator(buffer* buffer)
    : m_buffer(buffer)
  {}
  buffer_allocator(const buffer_allocator& o)
    : m_buffer(o.m_buffer)
  {}
  buffer_allocator(buffer_allocator&& o)
    : m_buffer(std::exchange(o.m_buffer, nullptr))
  {}

  template<typename U>
  buffer_allocator(buffer_allocator<U>& o)
    : m_buffer(o.m_buffer)
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
    if (this->m_buffer == nullptr)
      throw std::logic_error("erro: the allocator wasn't initialized with memory buffer");

    return reinterpret_cast<pointer>(this->m_buffer->allocate(size * sizeof(value_type)));
  };
  inline void
  deallocate(pointer p, size_type sz)
  {
    if (this->m_buffer == nullptr)
      throw std::logic_error("erro: the allocator wasn't initialized with memory buffer");

    this->m_buffer->deallocate(reinterpret_cast<char*>(p), sz);
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
  operator==(buffer_allocator const& a)
  {
    return this == &a || this->m_buffer == a.m_buffer;
  };
  inline bool
  operator!=(buffer_allocator const& a)
  {
    return !operator==(a);
  };
  buffer_allocator&
  operator=(buffer_allocator&& o)
  {
    if (this != &o) {
      this->m_buffer = std::exchange(o.m_buffer, nullptr);
    };

    return *this;
  };
};

class buffer_fx : public buffer
{
public:
  using value_type      = typename buffer::value_type;
  using pointer         = typename buffer::pointer;
  using const_pointer   = typename buffer::const_pointer;
  using size_type       = typename buffer::size_type;
  using difference_type = typename buffer::difference_type;

private:
  size_type                   m_alignment;
  size_type                   m_size;
  std::shared_ptr<value_type> m_mem;

  pointer m_cmem;

public:
  buffer_fx()
    : m_mem(nullptr)
    , m_cmem(nullptr)
    , m_size(size_type())
    , m_alignment(size_type())
  {
  }
  buffer_fx(std::shared_ptr<value_type> memory, size_type size, size_type alignment)
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
  operator==(buffer_fx const& a)
  {
    return this == &a || this->m_mem == a.m_mem && this->m_size == a.m_size;
  };
  inline bool
  operator!=(buffer_fx const& a)
  {
    return !operator==(a);
  };
};

class buffer_dyn : public buffer
{
public:
  using value_type      = typename buffer::value_type;
  using pointer         = typename buffer::pointer;
  using const_pointer   = typename buffer::const_pointer;
  using size_type       = typename buffer::size_type;
  using difference_type = typename buffer::difference_type;

private:
  std::vector<pointer> m_allocations;

  size_type m_alignment;

public:
  buffer_dyn()
    : m_alignment(size_type())
  {
  }
  ~buffer_dyn()
  {
    std::for_each(
      this->m_allocations.begin(), 
      this->m_allocations.end(), 
      [this](pointer p) -> void { 
        this->_deallocate(p, 0);
      });
  }

private:
  inline void
  _deallocate(pointer p, size_type)
  {
    if (this->m_alignment != 0) {
      _aligned_free(p);
    } else {
      free(p);
    }
  };

public:
  inline pointer
  allocate(size_type size)
  {
    pointer p;
    if (this->m_alignment != 0) {
      p = reinterpret_cast<pointer>(_aligned_malloc(size, this->m_alignment));
    } else {
      p = reinterpret_cast<pointer>(malloc(size));
    }
    if (p == nullptr)
      throw std::runtime_error("erro: can't allocate dynamic memory");

    this->m_allocations.push_back(p);

    return reinterpret_cast<pointer>(p);
  };
  inline void
  deallocate(pointer p, size_type)
  {
    auto it = std::find(this->m_allocations.begin(), this->m_allocations.end(), p);
    if (it != this->m_allocations.end()) {
      this->m_allocations.erase(it);

      this->_deallocate(p, 0);
    }
  };

  inline bool
  operator==(buffer_dyn const& a)
  {
    return this->m_alignment == a.m_alignment;
  };
  inline bool
  operator!=(buffer_dyn const& a)
  {
    return !operator==(a);
  };
};

} // namespace memory
} // namespace apsp

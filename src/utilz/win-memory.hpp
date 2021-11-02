#pragma once

// global includes
//
#include <memory>
#include <stdexcept>

// operating system includes
//
#include <windows.h>

// local operating system includes
//
#include "win-wrappers.hpp"

namespace utilz {
namespace memory {

void
initialize_large_pages()
{
  HANDLE           Token;
  TOKEN_PRIVILEGES TokenPrivileges;

  TokenPrivileges.PrivilegeCount           = 1UL;
  TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &Token))
    throw std::logic_error("");

  // Store 'Token' in a wrapper to ensure proper RAII
  //
  ::utilz::win_handle token_handle(Token);

  if (!::LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &TokenPrivileges.Privileges[0].Luid))
    throw std::logic_error("");

  if (!::AdjustTokenPrivileges(token_handle.native(), FALSE, &TokenPrivileges, 0UL, NULL, NULL))
    throw std::logic_error("");
}

template<typename T>
class large_pages_allocator
{
public:
  using value_type      = T;
  using pointer         = value_type*;
  using const_pointer   = const value_type*;
  using reference       = value_type&;
  using const_reference = const value_type&;
  using size_type       = std::size_t;
  using difference_type = std::ptrdiff_t;

public:
  template<typename U>
  struct rebind
  {
    using other = large_pages_allocator<U>;
  };

public:
  large_pages_allocator()
  {}

  template<typename U>
  inline large_pages_allocator(large_pages_allocator<U>& o)
  {}

  size_t
  page_size() noexcept
  {
    return static_cast<size_t>(::GetLargePageMinimum());
  };

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
  allocate(size_type size, typename std::allocator<void>::const_pointer = nullptr)
  {
    size_t granularity = ::GetLargePageMinimum();
    size_t pages = size * sizeof(T) / granularity;
    if (size * sizeof(T) < granularity)
      pages++;

    pointer p = reinterpret_cast<pointer>(
      ::VirtualAlloc(NULL, pages * granularity, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE));
      
    if (p == nullptr)
      throw std::bad_alloc();

    return p;
  };
  inline void
  deallocate(pointer p, size_type)
  {
    if (!::VirtualFree(p, 0, MEM_RELEASE))
      throw std::runtime_error("cannot release allocated virtual memory");
  };

  inline size_type
  max_size() const
  {
    return (std::numeric_limits<size_type>::max)() / sizeof(T);
  };

  inline void
  construct(pointer p, const T& t)
  {
    new (p) T(t);
  };
  inline void
  destroy(pointer p)
  {
    p->~T();
  };

  inline bool
  operator==(large_pages_allocator const&)
  {
    return true;
  };
  inline bool
  operator!=(large_pages_allocator const& a)
  {
    return !operator==(a);
  };
};

} // namespace memory
} // namespace apsp

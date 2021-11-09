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
__largepages_init()
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

void*
__largepages_malloc(size_t size)
{
  const DWORD flags = MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES;

  size_t granularity = ::GetLargePageMinimum();
  size_t pages       = size / granularity;
  if ((size < granularity) || (size % granularity != 0))
    pages++;

  return ::VirtualAlloc(NULL, pages * granularity, flags, PAGE_READWRITE);
};

void
__largepages_free(void* m)
{
  if (m != nullptr)
    ::VirtualFree(m, 0, MEM_RELEASE);
}

} // namespace memory
} // namespace apsp

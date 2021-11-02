#pragma once

// operating system includes
//
#include <windows.h>

namespace utilz {

class win_handle
{
private:
  HANDLE m_h;
  HANDLE m_invalid;

public:
  win_handle(HANDLE h)
    : m_h(h)
    , m_invalid(NULL)
  {
  }
  win_handle(HANDLE h, HANDLE invalid)
    : m_h(h)
    , m_invalid(invalid)
  {
  }

  ~win_handle()
  {
    if (this->valid()) {
      ::CloseHandle(this->m_h);

      this->m_h = this->m_invalid;
    }
  }

  bool
  valid()
  {
    return this->m_h != this->m_invalid;
  }

  HANDLE
  native()
  {
    return this->m_h;
  }

  win_handle&
  operator=(const win_handle& o) = delete;

  win_handle&
  operator=(win_handle&& o)
  {
    if (this != &o) {
      this->m_h       = o.m_h;
      this->m_invalid = o.m_invalid;

      o.m_h = o.m_invalid;
    };
    return *this;
  };
};

} // namespace utilz

#include <iostream>

#include <windows.h>

size_t get_native_mem_page_sz()
{
  SYSTEM_INFO Si;
  ::GetSystemInfo(&Si);

  return static_cast<size_t>(Si.dwPageSize);
}

size_t get_large_mem_page_sz()
{
  return static_cast<size_t>(::GetLargePageMinimum());
}

int
main(int argc, char* argv[])
{
  std::cout << "Supported memory page sizes" << std::endl;
  std::cout << "  " << get_native_mem_page_sz() << " B (native)" << std::endl
            << "  " << get_large_mem_page_sz()  << " B (large)" << std:: endl;
}


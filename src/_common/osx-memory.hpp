#pragma once

// global includes
//
#include <memory>

namespace utilz {
namespace memory {

void
__largepages_init()
{
}

void*
__largepages_malloc(size_t size)
{
  return ::malloc(size);
};

void
__largepages_free(void* m)
{
  if (m != nullptr)
    ::free(m);
}

} // namespace memory
} // namespace apsp

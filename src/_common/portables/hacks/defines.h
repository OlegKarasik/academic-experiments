#pragma once

#ifdef __INTEL_COMPILER
#define __hack_noinline __declspec(noinline)
#else
#define __hack_noinline __attribute__((noinline))
#endif

#ifdef __INTEL_COMPILER
#define __hack_noexcept
#else
#define __hack_noexcept noexcept
#endif

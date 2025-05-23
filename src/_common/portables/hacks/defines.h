#pragma once

#if defined(__INTEL_COMPILER) || defined(__INTEL_LLVM_COMPILER)
#define _INTEL_COMPILER
#endif

#ifdef _INTEL_COMPILER
#define __hack_noinline __declspec(noinline)
#else
#define __hack_noinline __attribute__((noinline))
#endif

#ifdef _INTEL_COMPILER
#else
#define __hack_nounroll _Pragma("GCC unroll 1")
#endif

#ifdef _INTEL_COMPILER
#define __hack_noexcept
#else
#define __hack_noexcept noexcept
#endif

#ifdef _INTEL_COMPILER
#define __hack_ivdep _Pragma("ivdep")
#else
#define __hack_ivdep _Pragma("GCC ivdep")
#endif

#pragma once

#if defined(__INTEL_COMPILER) || defined(__INTEL_LLVM_COMPILER)
#define _INTEL_COMPILER
#endif

// __hack_noinline

#if defined(_INTEL_COMPILER)
#define __hack_noinline __declspec(noinline)
#elif defined(__clang__)
#define __hack_noinline __attribute__((noinline))
#elif defined(__GNUC__)
#define __hack_noinline __attribute__((noinline))
#else
#define __hack_noinline
#endif

// __hack_noexcept

#if defined(_INTEL_COMPILER)
#define __hack_noexcept
#elif defined(__clang__)
#define __hack_noexcept noexcept
#elif defined(__GNUC__)
#define __hack_noexcept noexcept
#else
#define __hack_noexcept
#endif

// __hack_ivdep

#if defined(_INTEL_COMPILER)
#define __hack_ivdep _Pragma("ivdep")
#elif defined(__clang__)
#define __hack_ivdep _Pragma("clang loop vectorize(assume_safety)")
#elif defined(__GNUC__)
#define __hack_ivdep _Pragma("GCC ivdep")
#else
#define __hack_ivdep
#endif

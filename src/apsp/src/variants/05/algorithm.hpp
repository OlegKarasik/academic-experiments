#pragma once

#define APSP_ALG_MATRIX

#include "portables/hacks/defines.h"

#include "Kernel{Core}.h"
#include "Kernel{Algorithm}.h"
#include "Kernel{Base}.h"
#include "Kernel{Bitfield}.h"
#include "Kernel{Memory}.h"
#include "Kernel{Queue}.h"
#include "Kernel{System}.h"

#include "matrix.hpp"

template<typename T, typename A>
__hack_noinline void
run(
  ::utilz::matrices::square_matrix<T, A>& m)
{
  KRCORE_INIT_DATA CoreInitData = { 0 };
  CoreInitData.TraceKey = 1UL;

  PKRCORE Core = ::KrCoreInitialize(&CoreInitData);
};

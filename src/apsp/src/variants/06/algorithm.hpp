#pragma once

#define APSP_ALG_MATRIX

#include "portables/hacks/defines.h"

#include "matrix.hpp"

#include "metal-cpp/Metal.hpp"

template<typename T, typename A>
__hack_noinline void
run(
  ::utilz::matrices::square_matrix<T, A>& m)
{

  NS::Error *error = nullptr;
  auto library = NS::String::string("_application-v06-metal.metallib", NS::ASCIIStringEncoding);

  MTL::Device *device = MTL::CreateSystemDefaultDevice();
  MTL::Library *defaultLibrary = device->newLibrary(library, &error);
};

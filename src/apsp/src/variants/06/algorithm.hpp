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
  const unsigned long long arrayLength = 60 * 180 * 100000;
  const unsigned long long bufferSize = arrayLength * sizeof(float);

  NS::Error *error = nullptr;
  auto library_name = NS::String::string("_algorithm-v06-metal.metallib", NS::ASCIIStringEncoding);

  MTL::Device *device = MTL::CreateSystemDefaultDevice();
  MTL::Library *library = device->newLibrary(library_name, &error);

  auto function_name = NS::String::string("calculate", NS::ASCIIStringEncoding);
  MTL::Function *function = library->newFunction(function_name);

  //
  library->release();
  //

  MTL::ComputePipelineState *pipeline_state = device->newComputePipelineState(function, &error);

  //
  function->release();
  //

  MTL::Buffer *bufferA = device->newBuffer(m.size() * m.size() * sizeof(T), MTL::ResourceStorageModeShared);
  int* content = (int*)bufferA->contents();
  for (auto i = 0; i < m.size(); ++i)
  {
    for (auto j = 0; j < m.size(); ++j)
    {
      content[i * m.size() + j] = m.at(i, j);
    }
  }

  ///
  MTL::CommandQueue *command_queue = device->newCommandQueue();

  MTL::Size size = MTL::Size::Make(1, 1, 1);
  NS::UInteger group_size = pipeline_state->maxTotalThreadsPerThreadgroup();
  if (group_size > (m.size()+1))
  {
    group_size = m.size()+1;
  }
  MTL::Size group_final_size = MTL::Size::Make(group_size, 1, 1);

  using size_type = typename ::utilz::matrices::traits::matrix_traits<utilz::matrices::square_matrix<T, A>>::size_type;
  const auto x = m.size();
  for (auto k = size_type(0); k < x; ++k) {
    MTL::CommandBuffer *command_buffer = command_queue->commandBuffer();
    MTL::ComputeCommandEncoder *command_encoder = command_buffer->computeCommandEncoder();

    command_encoder->setComputePipelineState(pipeline_state);

    for (auto i = size_type(0); i < x; ++i) {
      command_encoder->setBuffer(bufferA, i * x * sizeof(T), 0);
      command_encoder->setBuffer(bufferA, k * x * sizeof(T), 1);
      command_encoder->setBytes(&k, sizeof(size_type), 2);
      command_encoder->dispatchThreadgroups(size, group_final_size);
    }

    command_encoder->endEncoding();
    command_buffer->commit();
    command_buffer->waitUntilCompleted();
    command_buffer->release();
  }

  for (auto i = 0; i < m.size(); ++i)
  {
    for (auto j = 0; j < m.size(); ++j)
    {
      m.at(i, j) = content[i * m.size() + j];
    }
  }

  bufferA->release();
};

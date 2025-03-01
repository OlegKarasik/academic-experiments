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
  auto library_name = NS::String::string("_application-v06-metal.metallib", NS::ASCIIStringEncoding);

  MTL::Device *device = MTL::CreateSystemDefaultDevice();
  MTL::Library *library = device->newLibrary(library_name, &error);

  auto function_name = NS::String::string("add_arrays", NS::ASCIIStringEncoding);
  MTL::Function *function = library->newFunction(function_name);

  //
  library->release();
  //

  MTL::ComputePipelineState *pipeline_state = device->newComputePipelineState(function, &error);

  //
  function->release();
  //



  MTL::Buffer *bufferA = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);
  MTL::Buffer *bufferB = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);
  MTL::Buffer *bufferR = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);

  auto contentA = (float*)bufferA->contents();
  auto contentB = (float*)bufferB->contents();
  auto contentR = (float*)bufferR->contents();

  for (auto i = 0; i < arrayLength; ++i)
  {
    contentA[i] = (float)rand() / (float)(RAND_MAX);
    contentB[i] = (float)rand() / (float)(RAND_MAX);
    contentR[i] = (float)0;
  }

  ///
  MTL::CommandQueue *command_queue = device->newCommandQueue();
  MTL::CommandBuffer *command_buffer = command_queue->commandBuffer();
  MTL::ComputeCommandEncoder *command_encoder = command_buffer->computeCommandEncoder();

  command_encoder->setComputePipelineState(pipeline_state);
  command_encoder->setBuffer(bufferA, 0, 0);
  command_encoder->setBuffer(bufferB, 0, 1);
  command_encoder->setBuffer(bufferR, 0, 2);

  MTL::Size size = MTL::Size::Make(arrayLength, 1, 1);
  NS::UInteger group_size = pipeline_state->maxTotalThreadsPerThreadgroup();
  if (group_size > arrayLength)
  {
    group_size = arrayLength;
  }
  MTL::Size group_final_size = MTL::Size::Make(group_size, 1, 1);

  command_encoder->dispatchThreadgroups(size, group_final_size);

  command_encoder->endEncoding();
  command_buffer->commit();
  command_buffer->waitUntilCompleted();
  ///

  for (auto i = 0; i < arrayLength; ++i)
  {
    if (contentR[i] != (contentA[i] + contentB[i]))
    {
      throw std::runtime_error("qew");
    }
  }

  bufferA->release();
  bufferB->release();
  bufferR->release();
};

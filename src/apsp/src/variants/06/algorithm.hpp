#pragma once

#define APSP_ALG_MATRIX

#define APSP_ALG_EXTRA_CONFIGURATION

#include "portables/hacks/defines.h"

#include "memory.hpp"
#include "matrix.hpp"
#include "constants.hpp"

#include "metal-cpp/Metal.hpp"

template<typename T>
struct run_configuration
{
  MTL::Device*               device;
  MTL::ComputePipelineState* pipeline_state;
  MTL::CommandQueue*         command_queue;
};

template<typename T, typename A>
__hack_noinline void
up(
  utilz::matrices::square_matrix<T, A>& m,
  utilz::memory::buffer& b,
  run_configuration<T>& run_config)
{
  auto library_name  = NS::String::string("_algorithm-v06-metal.metallib", NS::ASCIIStringEncoding);
  auto function_name = NS::String::string("calculate", NS::ASCIIStringEncoding);

  run_config.device = MTL::CreateSystemDefaultDevice();

  NS::Error*    error   = nullptr;
  MTL::Library* library = run_config.device->newLibrary(library_name, &error);
  if (library == nullptr)
    throw std::runtime_error(
      "Unable to load \"_algorithm-v06-metal.metallib\" library, please ensure working directory is set to binary location");

  MTL::Function* function = library->newFunction(function_name);
  if (function == nullptr)
    throw std::runtime_error(
      "Unable to load \"calculate\" function from \"_algorithm-v06-metal.metallib\"");

  // Release the library object because it is not needed anymore
  //
  library->release();

  run_config.pipeline_state = run_config.device->newComputePipelineState(function, &error);

  // Release the function object because it is not needed anymore
  //
  function->release();

  run_config.command_queue = run_config.device->newCommandQueue();
};

template<typename T, typename A>
__hack_noinline void
down(
  utilz::matrices::square_matrix<T, A>& m,
  utilz::memory::buffer& b,
  run_configuration<T>& run_config)
{
  run_config.command_queue->release();
  run_config.pipeline_state->release();
  run_config.device->release();
}

template<typename T, typename A>
__hack_noinline void
run(
  ::utilz::matrices::square_matrix<T, A>& m,
  run_configuration<T>& run_config)
{
  using size_type = typename ::utilz::matrices::traits::matrix_traits<utilz::matrices::square_matrix<T, A>>::size_type;
  const auto x = m.size();

  MTL::Buffer* memory = run_config.device->newBuffer(m.at(0), x * x * sizeof(T), MTL::ResourceStorageModeShared);

  // Copy matrix into dedicate buffer (shared across devices)
  //
  memcpy(memory->contents(), m.at(0), x * x * sizeof(T));

  MTL::Size grid_size  = MTL::Size::Make(x, x, 1);
  MTL::Size group_size = MTL::Size::Make(
    run_config.pipeline_state->maxTotalThreadsPerThreadgroup() > x
      ? run_config.pipeline_state->maxTotalThreadsPerThreadgroup()
      : x, 1, 1);


  MTL::CommandBuffer*         command_buffer  = run_config.command_queue->commandBuffer();
  MTL::ComputeCommandEncoder* command_encoder = command_buffer->computeCommandEncoder();

  command_encoder->setComputePipelineState(run_config.pipeline_state);

  for (auto k = size_type(0); k < x; ++k) {
    command_encoder->setBuffer(memory, 0, 0);
    command_encoder->setBytes(&x, sizeof(size_type), 1);
    command_encoder->setBytes(&k, sizeof(size_type), 2);
    command_encoder->dispatchThreads(grid_size, group_size);
  }

  command_encoder->endEncoding();
  command_buffer->commit();
  command_buffer->waitUntilCompleted();

  command_encoder->release();
  command_buffer->release();

  // Copy memory back from the shared buffer into the matrix
  //
  memcpy(m.at(0), memory->contents(), x * x * sizeof(T));

  memory->release();
};

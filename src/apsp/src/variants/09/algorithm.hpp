#pragma once

#define APSP_ALG_MATRIX

#define APSP_ALG_EXTRA_CONFIGURATION

#include "portables/hacks/defines.h"

#include "memory.hpp"
#include "matrix.hpp"
#include "matrix-access.hpp"
#include "constants.hpp"

#include "metal-cpp/Metal.hpp"

namespace utzmx = ::utilz::matrices;

template<typename T, typename A>
struct run_configuration
{
  MTL::Device*               device;
  MTL::ComputePipelineState* calculate_diagonal_pipeline_state;
  MTL::ComputePipelineState* calculate_cross_x_pipeline_state;
  MTL::ComputePipelineState* calculate_cross_y_pipeline_state;
  MTL::ComputePipelineState* calculate_peripheral_pipeline_state;
  MTL::CommandQueue*         command_queue;
};

template<typename T, typename A>
__hack_noinline
void
up(
  utzmx::matrix_abstract<utzmx::square_matrix<T, A>>& m,
  utilz::memory::buffer& b,
  run_configuration<T, A>& run_config)
{
  auto library_name  = NS::String::string("_algorithm-v09-metal.metallib", NS::ASCIIStringEncoding);

  auto calculate_diagonal_function_name   = NS::String::string("calculate_diagonal",   NS::ASCIIStringEncoding);
  auto calculate_cross_x_function_name    = NS::String::string("calculate_cross_x",    NS::ASCIIStringEncoding);
  auto calculate_cross_y_function_name    = NS::String::string("calculate_cross_y",    NS::ASCIIStringEncoding);
  auto calculate_peripheral_function_name = NS::String::string("calculate_peripheral", NS::ASCIIStringEncoding);

  run_config.device = MTL::CreateSystemDefaultDevice();

  NS::Error*    error   = nullptr;
  MTL::Library* library = run_config.device->newLibrary(library_name, &error);
  if (library == nullptr)
    throw std::runtime_error(
      "Unable to load \"_algorithm-v09-metal.metallib\" library, please ensure working directory is set to binary location");

  MTL::Function* calculate_diagonal_function   = library->newFunction(calculate_diagonal_function_name);
  if (calculate_diagonal_function == nullptr) throw std::runtime_error("Unable to load \"calculate_diagonal\" function from \"_algorithm-v09-metal.metallib\"");

  MTL::Function* calculate_cross_x_function    = library->newFunction(calculate_cross_x_function_name);
  if (calculate_cross_x_function == nullptr) throw std::runtime_error("Unable to load \"calculate_cross_x\" function from \"_algorithm-v09-metal.metallib\"");

  MTL::Function* calculate_cross_y_function    = library->newFunction(calculate_cross_y_function_name);
  if (calculate_cross_y_function == nullptr) throw std::runtime_error("Unable to load \"calculate_cross_y\" function from \"_algorithm-v09-metal.metallib\"");

  MTL::Function* calculate_peripheral_function = library->newFunction(calculate_peripheral_function_name);
  if (calculate_peripheral_function == nullptr) throw std::runtime_error("Unable to load \"calculate_peripheral\" function from \"_algorithm-v09-metal.metallib\"");

  // Release the library object because it is not needed anymore
  //
  library->release();

  run_config.calculate_diagonal_pipeline_state   = run_config.device->newComputePipelineState(calculate_diagonal_function, &error);
  run_config.calculate_cross_x_pipeline_state    = run_config.device->newComputePipelineState(calculate_cross_x_function, &error);
  run_config.calculate_cross_y_pipeline_state    = run_config.device->newComputePipelineState(calculate_cross_y_function, &error);
  run_config.calculate_peripheral_pipeline_state = run_config.device->newComputePipelineState(calculate_peripheral_function, &error);

  // Release the function object because it is not needed anymore
  //
  calculate_diagonal_function->release();
  calculate_cross_x_function->release();
  calculate_cross_y_function->release();
  calculate_peripheral_function->release();

  run_config.command_queue = run_config.device->newCommandQueue();
};

template<typename T, typename A>
__hack_noinline
void
down(
  utzmx::matrix_abstract<utzmx::square_matrix<T, A>>& m,
  utilz::memory::buffer& b,
  run_configuration<T, A>& run_config)
{
  run_config.command_queue->release();
  run_config.calculate_diagonal_pipeline_state->release();
  run_config.calculate_cross_x_pipeline_state->release();
  run_config.calculate_cross_y_pipeline_state->release();
  run_config.calculate_peripheral_pipeline_state->release();
  run_config.device->release();
}

template<typename T, typename A>
__hack_noinline
void
run(
  utzmx::square_matrix<T, A>& m,
  run_configuration<T, A>& run_config)
{
  using size_type = typename utzmx::traits::matrix_traits<utzmx::square_matrix<T, A>>::size_type;

  const auto sz = m.size();

  MTL::Buffer* memory = run_config.device->newBuffer(m.at(0), sz * sz * sizeof(T), MTL::ResourceStorageModeShared);

  // Copy matrix into dedicate buffer (shared across devices)
  //
  memcpy(memory->contents(), m.at(0), sz * sz * sizeof(T));

  MTL::Size grid_size  = MTL::Size::Make(sz, sz, 1);
  // MTL::Size group_size = MTL::Size::Make(
  //   run_config.pipeline_state->maxTotalThreadsPerThreadgroup() > sz
  //     ? sz
  //     : run_config.pipeline_state->maxTotalThreadsPerThreadgroup(),
  //   1,
  //   1);

  // MTL::CommandBuffer*         command_buffer  = run_config.command_queue->commandBuffer();
  // MTL::ComputeCommandEncoder* command_encoder = command_buffer->computeCommandEncoder();

  // MTL::CommandBuffer*         command_bufferx  = run_config.command_queue->commandBuffer();
  // MTL::ComputeCommandEncoder* command_encoderx = command_bufferx->computeCommandEncoder();

  // command_encoder->setComputePipelineState(run_config.pipeline_state);
  // command_encoderx->setComputePipelineState(run_config.pipeline_statex);

  // for (auto k = size_type(0); k < sz; ++k) {
  //   command_encoder->setBuffer(memory, 0, 0);
  //   command_encoder->setBytes(&sz, sizeof(size_type), 1);
  //   command_encoder->setBytes(&k, sizeof(size_type), 2);
  //   command_encoder->dispatchThreads(grid_size, group_size);

  //   command_encoderx->setBuffer(memory, 0, 0);
  //   command_encoderx->setBytes(&sz, sizeof(size_type), 1);
  //   command_encoderx->setBytes(&k, sizeof(size_type), 2);
  //   command_encoderx->dispatchThreads(grid_size, group_size);
  // }

  // command_encoder->endEncoding();
  // command_encoderx->endEncoding();
  // command_buffer->commit();
  // command_bufferx->commit();
  // command_buffer->waitUntilCompleted();
  // command_bufferx->waitUntilCompleted();

  // command_encoder->release();
  // command_encoderx->release();
  // command_buffer->release();
  // command_bufferx->release();

  // Copy memory back from the shared buffer into the matrix
  //
  memcpy(m.at(0), memory->contents(), sz * sz * sizeof(T));

  memory->release();
};

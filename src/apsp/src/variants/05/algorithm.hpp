#pragma once

#define APSP_ALG_MATRIX_BLOCKS

#define APSP_ALG_EXTRA_CONFIGURATION

#include "portables/hacks/defines.h"

#include "Kernel{Algorithm}.h"
#include "Kernel{Base}.h"
#include "Kernel{Bitfield}.h"
#include "Kernel{Core}.h"
#include "Kernel{Memory}.h"
#include "Kernel{Queue}.h"
#include "Kernel{System}.h"

#include "matrix.hpp"

#include <thread>
#include <cassert>

template<typename T, typename A, typename U>
struct stream_node
{
  using size_type = typename ::utilz::matrices::square_matrix<::utilz::matrices::square_matrix<T, A>, U>::size_type;

  size_type rank;
  size_type processors_count;

  ::utilz::matrices::square_matrix<::utilz::matrices::square_matrix<T, A>, U>* blocks;
  ::utilz::matrices::square_matrix<short, ::utilz::memory::buffer_allocator<short>>* heights;

  PPKRCORE_TASK      tasks;
  size_type syncblock_count;
  PPKRCORE_SYNCBLOCK syncblocks;
};

template<typename T, typename A, typename U>
struct run_configuration
{
  using size_type = typename ::utilz::matrices::square_matrix<utilz::matrices::square_matrix<T, A>, U>::size_type;

  ::utilz::matrices::square_matrix<short, ::utilz::memory::buffer_allocator<short>> heights;

  size_type tasks_count;
  size_type threads_count;
  size_type syncblock_count;

  PKRCORE            core;
  PPKRCORE_TASK      tasks;
  PPKRCORE_THREAD    threads;
  PPKRCORE_SYNCBLOCK syncblocks;
  PKRCORE_SYNCBLOCK  delay;

  stream_node<T, A, U>* nodes;
};


void
wait_block(
  ::utilz::matrices::square_matrix<short, ::utilz::memory::buffer_allocator<short>>* heights,
  size_t syncblocks_count,
  PPKRCORE_SYNCBLOCK syncblocks,
  size_t rank,
  size_t block_index,
  size_t row,
  size_t col)
{
  PKRCORE_SYNCBLOCK s = syncblocks[(row + col) % syncblocks_count];

  short v = static_cast<short>(block_index + 1ULL);

  while (heights->at(row, col) < v)
    KRASSERT(::KrCoreTaskCurrentSynchronize(s));
};

void
notify_block(
  ::utilz::matrices::square_matrix<short, ::utilz::memory::buffer_allocator<short>>* heights,
  size_t syncblocks_count,
  PPKRCORE_SYNCBLOCK syncblocks,
  size_t progress_value,
  size_t row,
  size_t col)
{
  PKRCORE_SYNCBLOCK s = syncblocks[(row + col) % syncblocks_count];

  heights->at(row, col) = static_cast<short>(progress_value + 1ULL);

  KRASSERT(::KrCoreSyncblockSignal(s));
};

template<typename T, typename A, typename U>
void
calculate_block(
  ::utilz::matrices::square_matrix<T, A>& ij,
  ::utilz::matrices::square_matrix<T, A>& ik,
  ::utilz::matrices::square_matrix<T, A>& kj)
{
  using size_type = typename ::utilz::matrices::square_matrix<::utilz::matrices::square_matrix<T, A>, U>::size_type;

  const auto x = ij.size();
  for (auto k = size_type(0); k < x; ++k)
    for (auto i = size_type(0); i < x; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

template<typename T, typename A, typename U>
void
calculate_block_auto(
  ::utilz::matrices::square_matrix<::utilz::matrices::square_matrix<T, A>, U>* matrix,
  size_t                                                                       block_index,
  size_t                                                                       i,
  size_t                                                                       j)
{
  auto& ij = matrix->at(i, j);
  if (block_index != i) {
    if (block_index == j) {
      auto& current = matrix->at(block_index, block_index);

      calculate_block<T, A, U>(ij, ij, current);
    } else {
      auto& horizontal = matrix->at(i, block_index);
      auto& vertical   = matrix->at(block_index, j);

      calculate_block<T, A, U>(ij, horizontal, vertical);
    };
  } else if (block_index != j) {
    auto& current = matrix->at(block_index, block_index);

    calculate_block<T, A, U>(ij, current, ij);
  } else {
    calculate_block<T, A, U>(ij, ij, ij);
  };
};

template<typename T, typename A, typename U>
unsigned long
calculation_routine(
  void* routine_state
)
{
  using size_type = typename ::utilz::matrices::square_matrix<::utilz::matrices::square_matrix<T, A>, U>::size_type;

  auto* node   = reinterpret_cast<stream_node<T, A, U>*>(routine_state);
  auto* tasks = node->tasks;

//   auto& kr_task_index        = *std::get<0>(*p_kr_task_state);
//   auto& kr_stream_node_index = *std::get<1>(*p_kr_task_state);
//   auto& kr_stream_node       = *std::get<2>(*p_kr_task_state);
//   auto& kr_stream            = *kr_stream_node.mp_stream;

//   auto* block_matrix          = kr_stream.m_block_matrix.get_pointer();
//   auto* block_progress_matrix = kr_stream.m_progress_matrix.get_pointer();

//   const size_t block_size  = kr_stream.m_block_size;
//   const size_t block_count = kr_stream.m_block_count;

  const size_t processor_count = node->processors_count;
  const size_t rank            = node->rank;

  auto* blocks = node->blocks;
  auto* heights = node->heights;

  const size_t kr_top_task      = rank % processor_count;
  const size_t kr_bottom_task   = blocks->size() - (processor_count - kr_top_task);
  const size_t kr_previous_task = rank - processor_count;
  const size_t kr_next_task     = rank + processor_count;

  const bool move_top    = rank != kr_top_task;
  const bool move_bottom = rank != kr_bottom_task;

  size_t kr_src_task = kr_top_task;

  bool is_leader     = false;
  bool is_follower   = rank < processor_count;
  bool is_normalized = false;

  for (auto j = size_type(0); j < blocks->size(); ++j) {
    if (rank <= j) {
      for (auto block_index = size_type(0); block_index < rank; ++block_index) {
        wait_block(heights, node->syncblock_count, node->syncblocks, rank, block_index, block_index, j);

        calculate_block_auto(blocks, block_index, rank, j);
      };
      calculate_block_auto(blocks, rank, rank, j);

      notify_block(heights, node->syncblock_count, node->syncblocks, rank, rank, j);
    } else {
      for (size_t block_index = 0ULL; block_index <= j; ++block_index) {
        wait_block(heights, node->syncblock_count, node->syncblocks, rank, block_index, block_index, j);

        calculate_block_auto(blocks, block_index, rank, j);
      };
    };
    if (!is_leader) {
      if (rank == j) {
        for (auto reverse_j = size_type(0); reverse_j < j; ++reverse_j) {
          calculate_block_auto(blocks, rank, rank, reverse_j);

          notify_block(heights, node->syncblock_count, node->syncblocks, rank, rank, reverse_j);
        };

        for (auto forward_j = j + size_type(1); forward_j < blocks->size(); ++forward_j) {
          for (auto block_index = size_type(0); block_index < rank; ++block_index) {
            wait_block(heights, node->syncblock_count, node->syncblocks, rank, block_index, block_index, forward_j);

            calculate_block_auto(blocks, block_index, rank, forward_j);
          };
          calculate_block_auto(blocks, rank, rank, forward_j);

          notify_block(heights, node->syncblock_count, node->syncblocks, rank, rank, forward_j);
        };

        if (move_top)
          KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_top_task]));

        if (move_bottom)
          KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_next_task]));

        is_leader = true;
        break;
      } else {
        if (!is_follower) {
          if (move_bottom) {
            KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_next_task]));
          } else {
            KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_src_task]));

            if (j == kr_src_task)
              kr_src_task += processor_count;
          };
          is_follower = is_follower || (j == kr_previous_task);
          if (is_follower && !is_normalized) {
            for (size_t reverse_j = 0ULL; reverse_j < j; ++reverse_j) {
              for (size_t block_index = (reverse_j + 1UL); block_index <= j; ++block_index) {
                calculate_block_auto(blocks, block_index, rank, reverse_j);
              };
            };
            is_normalized = true;
          };
        } else {
          if (move_top)
            KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_top_task]));

          if (move_bottom)
            KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_next_task]));

          for (size_t reverse_j = 0ULL; reverse_j < j; ++reverse_j) {
            wait_block(heights, node->syncblock_count, node->syncblocks, rank, j, j, reverse_j);

            calculate_block_auto(blocks, j, rank, reverse_j);
          };
        };
      };
    };
  };

  is_leader   = false;
  is_follower = false;
  if (move_bottom) {
    KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_next_task]));

    for (size_t j = (rank + 1ULL); j <= kr_bottom_task; ++j) {
      for (size_t block_index = (rank + 1ULL); block_index <= j; ++block_index) {
        calculate_block_auto(blocks, block_index, rank, j);
      };

      KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_next_task]));
    };

    for (size_t reverse_j = 0ULL; reverse_j <= rank; ++reverse_j) {
      for (size_t block_index = (rank + 1ULL); block_index <= kr_bottom_task; ++block_index) {
        calculate_block_auto(blocks, block_index, rank, reverse_j);
      };
    };
    for (size_t reverse_j = (rank + 1ULL); reverse_j < kr_bottom_task; ++reverse_j) {
      for (size_t block_index = (reverse_j + 1ULL); block_index <= kr_bottom_task; ++block_index) {
        calculate_block_auto(blocks, block_index, rank, reverse_j);
      };
    };
    for (size_t reverse_j = (kr_bottom_task + 1ULL); reverse_j < blocks->size(); ++reverse_j) {
      for (size_t block_index = (rank + 1ULL); block_index <= kr_bottom_task; ++block_index) {
        calculate_block_auto(blocks, block_index, rank, reverse_j);
      };
    };

    KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_next_task]));

    for (size_t reverse_j = (kr_bottom_task + 1ULL); reverse_j < blocks->size(); ++reverse_j) {
      calculate_block_auto(blocks, reverse_j, rank, reverse_j);

      KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_next_task]));

      for (size_t inner_j = 0ULL; inner_j < reverse_j; ++inner_j) {
        calculate_block_auto(blocks, reverse_j, rank, inner_j);
      };
      for (size_t inner_j = (reverse_j + 1ULL); inner_j < blocks->size(); ++inner_j) {
        calculate_block_auto(blocks, reverse_j, rank, inner_j);
      };

      KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_next_task]));
    };
  } else {
    KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_top_task]));

    for (size_t reverse_j = (kr_bottom_task + 1ULL); reverse_j < blocks->size(); ++reverse_j) {
      wait_block(heights, node->syncblock_count, node->syncblocks, rank, reverse_j, reverse_j, reverse_j);

      calculate_block_auto(blocks, reverse_j, rank, reverse_j);

      KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_top_task]));

      for (size_t inner_j = 0ULL; inner_j < reverse_j; ++inner_j) {
        wait_block(heights, node->syncblock_count, node->syncblocks, rank, reverse_j, reverse_j, inner_j);

        calculate_block_auto(blocks, reverse_j, rank, inner_j);
      };
      for (size_t inner_j = (reverse_j + 1ULL); inner_j < blocks->size(); ++inner_j) {
        wait_block(heights, node->syncblock_count, node->syncblocks, rank, reverse_j, reverse_j, inner_j);

        calculate_block_auto(blocks, reverse_j, rank, inner_j);
      };

      KRASSERT(::KrCoreTaskCurrentSwitchToTask(tasks[kr_top_task]));
    };
  };

  if (rank == kr_bottom_task) {
    for (size_t i = kr_top_task; i < kr_bottom_task; i += processor_count)
      KRASSERT(::KrCoreTaskContinue(tasks[i]));
  };

  return 0UL;
};

template<typename T, typename A, typename U>
__hack_noinline
void
up(
  ::utilz::matrices::square_matrix<::utilz::matrices::square_matrix<T, A>, U>& blocks,
  ::utilz::memory::buffer& b,
  run_configuration<T, A, U>& run_config)
{
  using size_type = typename ::utilz::matrices::traits::matrix_traits<utilz::matrices::square_matrix<utilz::matrices::square_matrix<T, A>, U>>::size_type;

  // Set parameters
  //
  run_config.tasks_count     = blocks.size();
  run_config.threads_count   = size_type(std::thread::hardware_concurrency());
  run_config.syncblock_count = size_type(1000);

  // Initialise "Heights" matrix
  //
  run_config.heights = ::utilz::matrices::square_matrix<short, ::utilz::memory::buffer_allocator<short>>(
    run_config.tasks_count, ::utilz::memory::buffer_allocator<short>(&b));

  // Allocate space
  //
  run_config.tasks      = reinterpret_cast<PPKRCORE_TASK>(b.allocate(sizeof(PKRCORE_TASK) * run_config.tasks_count));
  run_config.threads    = reinterpret_cast<PPKRCORE_THREAD>(b.allocate(sizeof(PKRCORE_THREAD) * run_config.threads_count));
  run_config.syncblocks = reinterpret_cast<PPKRCORE_SYNCBLOCK>(b.allocate(sizeof(PKRCORE_SYNCBLOCK) * run_config.syncblock_count));
  run_config.nodes      = reinterpret_cast<stream_node<T, A, U>*>(b.allocate(sizeof(stream_node<T, A, U>) * run_config.tasks_count));

  // Prepare runtime configuration
  //
  for (auto i = size_type(0); i < blocks.size(); ++i) {
    run_config.nodes[i].rank = i;
    run_config.nodes[i].processors_count = std::thread::hardware_concurrency();
    run_config.nodes[i].tasks = run_config.tasks;
    run_config.nodes[i].syncblock_count = run_config.syncblock_count;
    run_config.nodes[i].syncblocks = run_config.syncblocks;

    run_config.nodes[i].blocks = &blocks;
    run_config.nodes[i].heights = &run_config.heights;
  }

  // Initialise Core
  //
  KRCORE_INIT_DATA CoreInitData = { 0 };
  CoreInitData.MaxResourceCount = 1 + /* delay syncblock */
    run_config.tasks_count +
    run_config.threads_count +
    run_config.syncblock_count;

  run_config.core = ::KrCoreInitialize(&CoreInitData);
  assert(run_config.core != nullptr);

  // Initialise delay Syncblock
  //
  KRCORE_SYNCBLOCK_INIT_DATA DelayInitData = { 0 };
  DelayInitData.ResetMode = CoreSyncblockResetModeManual;

  run_config.delay = ::KrCoreInitializeSyncblock(run_config.core, &DelayInitData);
  assert(run_config.delay != nullptr);

  // Initialise Syncblocks
  //
  for (auto i = size_type(0); i < run_config.syncblock_count; ++i) {
    KRCORE_SYNCBLOCK_INIT_DATA SyncblockInitData = { 0 };

    SyncblockInitData.ResetMode = CoreSyncblockResetModeAutoAll;

    run_config.syncblocks[i] = ::KrCoreInitializeSyncblock(run_config.core, &SyncblockInitData);

    assert(run_config.syncblocks[i] != nullptr);
  }

  // Initialise Threads
  //
  for (auto i = size_type(0); i < run_config.threads_count; ++i) {
    KRCORE_THREAD_INIT_DATA ThreadInitData = { 0 };

    ThreadInitData.Binding.Type = SystemBindingLogicalProcessors;
    ThreadInitData.Binding.LogicalProcessors.Group = 0;
    ThreadInitData.Binding.LogicalProcessors.Mask  = 1ULL << i;

    run_config.threads[i] = ::KrCoreInitializeThread(run_config.core, &ThreadInitData);

    assert(run_config.threads[i] != nullptr);
  }

  // Initialise Tasks
  //
  for (auto i = size_type(0); i < run_config.tasks_count; ++i) {
    KRCORE_TASK_INIT_DATA TaskInitData = { 0 };

    KRCORE_TASK_ATTRIBUTE Attributes[1];
    Attributes[0].Attribute          = CoreTaskAttributeStartupCreateDelayed;
    Attributes[0].ON_DELAY.Syncblock = run_config.delay;

    TaskInitData.AttributesCount = 1;
    TaskInitData.Attributes = Attributes;
    TaskInitData.Binding.Type = SystemBindingLogicalProcessors;
    TaskInitData.Binding.LogicalProcessors.Group = 0;
    TaskInitData.Binding.LogicalProcessors.Mask  = 1ULL << (i % run_config.threads_count);
    TaskInitData.TaskRoutineState = &run_config.nodes[i];
    TaskInitData.TaskRoutine = calculation_routine<T, A, U>;

    run_config.tasks[i] = ::KrCoreInitializeTask(run_config.core, &TaskInitData);

    assert(run_config.tasks[i] != nullptr);
  }
};

template<typename T, typename A, typename U>
__hack_noinline
void
down(
  ::utilz::matrices::square_matrix<::utilz::matrices::square_matrix<T, A>, U>& blocks,
  ::utilz::memory::buffer& b,
  run_configuration<T, A, U>& run_config)
{
  using size_type = typename ::utilz::matrices::traits::matrix_traits<utilz::matrices::square_matrix<utilz::matrices::square_matrix<T, A>, U>>::size_type;

  for (auto i = size_type(0); i < run_config.tasks_count; ++i)
    KRASSERT(::KrCoreDeinitializeTask(run_config.core, run_config.tasks[i]));

  for (auto i = size_type(0); i < run_config.threads_count; ++i)
    KRASSERT(::KrCoreDeinitializeThread(run_config.core, run_config.threads[i]));

  for (auto i = size_type(0); i < run_config.syncblock_count; ++i)
    KRASSERT(::KrCoreDeinitializeSyncblock(run_config.core, run_config.syncblocks[i]));

  KRASSERT(::KrCoreDeinitialize(run_config.core));
}

template<typename T, typename A, typename U>
__hack_noinline
void
run(
  ::utilz::matrices::square_matrix<::utilz::matrices::square_matrix<T, A>, U>& blocks,
  run_configuration<T, A, U>& run_config)
{
  // Unblock all previously initialised Tasks
  //
  KRASSERT(::KrCoreSyncblockSignal(run_config.delay));

  // Wait them run to completion
  //
  for (auto i = 0; i < run_config.tasks_count; ++i)
    ::WaitForSingleObject(run_config.tasks[i]->InternalThread, INFINITE);
};

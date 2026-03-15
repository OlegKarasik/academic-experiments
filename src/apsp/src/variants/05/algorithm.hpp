#pragma once

#define APSP_ALG_MATRIX_BLOCKS

#define APSP_ALG_ACCESS_BLOCKS

#define APSP_ALG_RUN_CONFIGURATION

#include "portables/hacks/defines.h"

#include "Kernel{Algorithm}.h"
#include "Kernel{Base}.h"
#include "Kernel{Bitfield}.h"
#include "Kernel{Core}.h"
#include "Kernel{Memory}.h"
#include "Kernel{Queue}.h"
#include "Kernel{System}.h"

#include "matrix.hpp"
#include "memory.hpp"

#include <thread>
#include <cassert>

namespace utzmx = ::utilz::matrices;

template<typename S>
struct run_configuration;

using matrix_block_type      = utzmx::square_matrix<g_type, g_allocator_type<g_type>>;
using matrix_type            = utzmx::square_matrix<matrix_block_type, g_allocator_type<matrix_block_type>>;
using matrix_access_type     = utzmx::access::matrix_access<utzmx::access::matrix_access_schema_flat, matrix_type>;
using matrix_params_type     = utzmx::access::matrix_params<matrix_type>;
using matrix_run_config_type = run_configuration<matrix_type>;

using heights_matrix      = utzmx::square_matrix<size_t, ::utilz::memory::buffer_allocator<size_t>>;
using heights_sync_matrix = utzmx::square_matrix<PKRCORE_SYNCBLOCK, ::utilz::memory::buffer_allocator<PKRCORE_SYNCBLOCK>>;

struct stream_node_wait_condition
{
  size_t* c;
  size_t  v;
};

template<typename S>
struct stream_node
{
  S* blocks;

  size_t rank;
  size_t processors_count;

  heights_matrix*      heights;
  heights_sync_matrix* heights_sync;

  PPKRCORE_TASK tasks;

  size_t fst_rank;
  size_t lst_rank;
  size_t nxt_rank;
  size_t prv_rank;
};

template<typename S>
struct run_configuration
{
  heights_matrix      heights;
  heights_sync_matrix heights_sync;

  size_t tasks_count;
  size_t threads_count;

  PKRCORE           core;
  PPKRCORE_TASK     tasks;
  PPKRCORE_THREAD   threads;
  PKRCORE_SYNCBLOCK delay;

  stream_node<S>* nodes;
};

void
wait_block(
  heights_matrix* heights,
  heights_sync_matrix* heights_sync,
  size_t height,
  size_t i,
  size_t j)
{
  stream_node_wait_condition wait_condition;
  wait_condition.c = &heights->at(i, j);
  wait_condition.v = height + size_t(1);

  if (*wait_condition.c >= wait_condition.v)
    return;

  std::ignore = ::KrCoreTaskCurrentSynchronizeUntil(
    heights_sync->at(i, j),
    [](void* state) -> BOOL
    {
      auto condition = (stream_node_wait_condition*)state;
      return *condition->c >= condition->v;
    },
    &wait_condition);
};

void
notify_block(
  heights_matrix* heights,
  heights_sync_matrix* heights_sync,
  size_t height,
  size_t i,
  size_t j)
{
  heights->at(i, j) = height + size_t(1);

  std::ignore = ::KrCoreSyncblockSignal(heights_sync->at(i, j));
};

void
calculate_block(
  matrix_block_type& ij,
  matrix_block_type& ik,
  matrix_block_type& kj)
{
  using size_type = typename utzmx::traits::matrix_traits<matrix_block_type>::size_type;

  const auto x = ij.size();
  for (auto k = size_type(0); k < x; ++k)
    for (auto i = size_type(0); i < x; ++i)
      __hack_ivdep
      for (auto j = size_type(0); j < x; ++j)
        ij.at(i, j) = (std::min)(ij.at(i, j), ik.at(i, k) + kj.at(k, j));
};

template<typename S>
__hack_noinline
void
calculate_complimenting_type(
  stream_node<S>* node
) {
  auto* tasks        = node->tasks;
  auto* blocks       = node->blocks;
  auto* heights      = node->heights;
  auto* heights_sync = node->heights_sync;

  const size_t p = node->processors_count;
  const size_t c = node->rank;

  const size_t fst_task = node->fst_rank;

  const bool move_top = c != fst_task;

  if (move_top)
    std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[fst_task]);

  for (auto j = c + size_t(1); j < blocks->size(); ++j) {
    auto& cj = blocks->at(c, j);

    wait_block(heights, heights_sync, j, j, j);
    calculate_block(cj, cj, blocks->at(j, j));

    if (move_top)
      std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[fst_task]);

    for (auto b = size_t(0); b < j; ++b) {
      wait_block(heights, heights_sync, j, j, b);
      calculate_block(blocks->at(c, b), cj, blocks->at(j, b));
    };
    for (auto b = j + size_t(1); b < blocks->size(); ++b) {
      wait_block(heights, heights_sync, j, j, b);
      calculate_block(blocks->at(c, b), cj, blocks->at(j, b));
    };

    if (move_top)
      std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[fst_task]);
  };

  for (size_t i = fst_task; i < c; i += p)
    std::ignore = ::KrCoreTaskContinue(tasks[i]);
}

template<typename S>
__hack_noinline
void
calculate_passive_type_c(
  stream_node<S>* node
) {
  auto* tasks  = node->tasks;
  auto* blocks = node->blocks;

  const size_t c = node->rank;

  const size_t lst_task = node->lst_rank;
  const size_t nxt_task = node->nxt_rank;

  const bool move_bottom = c != lst_task;

  for (auto j = size_t(0); j <= c; ++j) {
    auto& cj = blocks->at(c, j);

    for (auto b = c + size_t(1); b <= lst_task; ++b)
      calculate_block(cj, blocks->at(c, b), blocks->at(b, j));
  };

  for (auto j = c + size_t(1); j < lst_task; ++j) {
    auto& cj = blocks->at(c, j);

    for (auto b = j + size_t(1); b <= lst_task; ++b)
      calculate_block(cj, blocks->at(c, b), blocks->at(b, j));
  };

  for (auto j = lst_task + size_t(1); j < blocks->size(); ++j) {
    auto& cj = blocks->at(c, j);

    for (auto b = c + size_t(1); b <= lst_task; ++b)
      calculate_block(cj, blocks->at(c, b), blocks->at(b, j));
  };

  if (move_bottom)
    std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[nxt_task]);

  for (auto j = (lst_task + size_t(1)); j < blocks->size(); ++j) {
    auto& cj = blocks->at(c, j);
    auto& jj = blocks->at(j, j);

    calculate_block(cj, cj, jj);

    if (move_bottom)
      std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[nxt_task]);

    for (auto b = size_t(0); b < j; ++b)
      calculate_block(blocks->at(c, b), cj, blocks->at(j, b));

    for (auto b = (j + size_t(1)); b < blocks->size(); ++b)
      calculate_block(blocks->at(c, b), cj, blocks->at(j, b));

    if (move_bottom)
      std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[nxt_task]);
  };
}

template<typename S>
__hack_noinline
void
calculate_passive_type_b(
  stream_node<S>* node
) {
  auto* tasks  = node->tasks;
  auto* blocks = node->blocks;

  const size_t c = node->rank;

  const size_t lst_task = node->lst_rank;
  const size_t nxt_task = node->nxt_rank;

  const bool move_bottom = c != lst_task;

  if (move_bottom)
    std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[nxt_task]);

  for (size_t j = c + size_t(1); j <= lst_task; ++j) {
    auto& cj = blocks->at(c, j);

    for (auto b = c + size_t(1); b <= j; ++b)
      calculate_block(cj, blocks->at(c, b), blocks->at(b, j));

    if (move_bottom)
      std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[nxt_task]);
  };

  calculate_passive_type_c(node);
}

template<typename S>
__hack_noinline
void
calculate_leading_type(
  stream_node<S>* node
) {
  auto* tasks        = node->tasks;
  auto* blocks       = node->blocks;
  auto* heights      = node->heights;
  auto* heights_sync = node->heights_sync;

  const size_t c = node->rank;

  const size_t fst_task = node->fst_rank;
  const size_t lst_task = node->lst_rank;
  const size_t nxt_task = node->nxt_rank;

  const bool move_top    = c != fst_task;
  const bool move_bottom = c != lst_task;

  auto& cc = blocks->at(c, c);

  for (auto j = size_t(0); j < c; ++j) {
    auto& cj = blocks->at(c, j);

    calculate_block(cj, cc, cj);
    notify_block(heights, heights_sync, c, c, j);
  };

  for (auto j = c + size_t(1); j < blocks->size(); ++j) {
    auto& cj = blocks->at(c, j);

    for (auto b = size_t(0); b < c; ++b) {
      wait_block(heights, heights_sync, b, b, j);
      calculate_block(cj, blocks->at(c, b), blocks->at(b, j));
    };

    calculate_block(cj, cc, cj);
    notify_block(heights, heights_sync, c, c, j);
  };

  if (move_top)
    std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[fst_task]);

  if (move_bottom) {
    std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[nxt_task]);

    calculate_passive_type_b(node);
  } else {
    calculate_complimenting_type(node);
  };
}

template<typename S>
__hack_noinline
void
calculate_following_type(
  stream_node<S>* node
) {
  auto* tasks        = node->tasks;
  auto* blocks       = node->blocks;
  auto* heights      = node->heights;
  auto* heights_sync = node->heights_sync;

  const size_t p  = node->processors_count;
  const size_t c  = node->rank;

  const size_t fst_task = node->fst_rank;
  const size_t prv_task = node->prv_rank;
  const size_t lst_task = node->lst_rank;
  const size_t nxt_task = node->nxt_rank;

  const bool move_top    = c != fst_task;
  const bool move_bottom = c != lst_task;

  for (auto j = c >= p ? prv_task + size_t(1) : size_t(0); j < c; ++j) {
    auto& cj = blocks->at(c, j);

    for (auto b = size_t(0); b <= j; ++b) {
      wait_block(heights, heights_sync, b, b, j);
      calculate_block(cj, blocks->at(c, b), blocks->at(b, j));
    };

    if (move_top)
      std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[fst_task]);

    if (move_bottom)
      std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[nxt_task]);

    for (auto b = size_t(0); b < j; ++b) {
      wait_block(heights, heights_sync, j, j, b);
      calculate_block(blocks->at(c, b), cj, blocks->at(j, b));
    };
  };

  auto& cc = blocks->at(c, c);

  for (auto b = size_t(0); b < c; ++b) {
    wait_block(heights, heights_sync, b, b, c);
    calculate_block(cc, blocks->at(c, b), blocks->at(b, c));
  };

  calculate_block(cc, cc, cc);
  notify_block(heights, heights_sync, c, c, c);

  calculate_leading_type(node);
}

template<typename S>
__hack_noinline
void
calculate_passive_type_a(
  stream_node<S>* node
) {
  auto* tasks  = node->tasks;
  auto* blocks = node->blocks;

  const size_t p = node->processors_count;
  const size_t c = node->rank;

  const size_t fst_task = node->fst_rank;
  const size_t prv_task = node->prv_rank;
  const size_t lst_task = node->lst_rank;
  const size_t nxt_task = node->nxt_rank;

  const bool move_top    = c != fst_task;
  const bool move_bottom = c != lst_task;

  for (auto j = size_t(0), s = fst_task; j <= prv_task; ++j) {
    auto& cj = blocks->at(c, j);

    for (auto b = size_t(0); b <= j; ++b)
      calculate_block(cj, blocks->at(c, b), blocks->at(b, j));

    if (move_bottom) {
      std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[nxt_task]);
    } else {
      if (move_top) {
        std::ignore = ::KrCoreTaskCurrentSwitchToTask(tasks[s]);

        if (j == s) {
          s += p;
        }
      }
    };
  };
  for (size_t j = size_t(0); j < prv_task; ++j) {
    auto& cj = blocks->at(c, j);
    for (size_t b = j + size_t(1); b <= prv_task; ++b)
      calculate_block(cj, blocks->at(c, b), blocks->at(b, j));
  };

  calculate_following_type(node);
}

__hack_noinline
unsigned long
calculation_routine(
  void* routine_state
)
{
  auto* node = reinterpret_cast<stream_node<matrix_type>*>(routine_state);

  const size_t p = node->processors_count;
  const size_t c = node->rank;

  if (c < p)
  {
    calculate_following_type(node);
  }
  else
  {
    calculate_passive_type_a(node);
  }

  return 0UL;
};

__hack_noinline
void
up(
  matrix_type&        matrix,
  matrix_access_type& matrix_access,
  matrix_run_config_type&  matrix_run_config,
  ::utilz::memory::buffer& b)
{
  // Set parameters
  //
  matrix_run_config.tasks_count   = matrix.size();
  matrix_run_config.threads_count = matrix.size() > std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : matrix.size();

  // Initialise "Heights" matrix
  //
  matrix_run_config.heights      = heights_matrix(matrix_run_config.tasks_count, ::utilz::memory::buffer_allocator<size_t>(&b));
  matrix_run_config.heights_sync = heights_sync_matrix(matrix_run_config.tasks_count, ::utilz::memory::buffer_allocator<PKRCORE_SYNCBLOCK>(&b));

  // Allocate space
  //
  matrix_run_config.tasks   = reinterpret_cast<PPKRCORE_TASK>(b.allocate(sizeof(PKRCORE_TASK) * matrix_run_config.tasks_count));
  matrix_run_config.threads = reinterpret_cast<PPKRCORE_THREAD>(b.allocate(sizeof(PKRCORE_THREAD) * matrix_run_config.threads_count));
  matrix_run_config.nodes   = reinterpret_cast<stream_node<matrix_type>*>(b.allocate(sizeof(stream_node<matrix_type>) * matrix_run_config.tasks_count));

  // Prepare runtime configuration
  //
  for (auto i = size_t(0); i < matrix.size(); ++i) {
    matrix_run_config.nodes[i].rank = i;
    matrix_run_config.nodes[i].processors_count = matrix_run_config.threads_count;
    matrix_run_config.nodes[i].tasks = matrix_run_config.tasks;

    matrix_run_config.nodes[i].blocks = &matrix;
    matrix_run_config.nodes[i].heights = &matrix_run_config.heights;
    matrix_run_config.nodes[i].heights_sync = &matrix_run_config.heights_sync;

    matrix_run_config.nodes[i].fst_rank = i % matrix_run_config.threads_count;
    matrix_run_config.nodes[i].lst_rank = (matrix_run_config.tasks_count / matrix_run_config.threads_count) * matrix_run_config.threads_count + (i % matrix_run_config.threads_count);
    matrix_run_config.nodes[i].prv_rank = i - matrix_run_config.threads_count;
    matrix_run_config.nodes[i].nxt_rank = i + matrix_run_config.threads_count;

    if (i < matrix_run_config.threads_count)
      matrix_run_config.nodes[i].prv_rank = i;

    if (matrix_run_config.nodes[i].nxt_rank >= matrix_run_config.tasks_count)
      matrix_run_config.nodes[i].nxt_rank = i;

    if (matrix_run_config.threads_count > matrix_run_config.tasks_count)
      matrix_run_config.nodes[i].lst_rank = i;

    if (matrix_run_config.nodes[i].lst_rank < matrix_run_config.nodes[i].nxt_rank || matrix_run_config.nodes[i].lst_rank >= matrix_run_config.tasks_count)
      matrix_run_config.nodes[i].lst_rank = matrix_run_config.nodes[i].nxt_rank;
  }

  // Initialise Core
  //
  KRCORE_INIT_DATA CoreInitData;
  ::memset(&CoreInitData, 0, sizeof(KRCORE_INIT_DATA));

  CoreInitData.MaxResourceCount = 1 + /* delay syncblock */
    matrix_run_config.tasks_count +
    matrix_run_config.threads_count +
    matrix_run_config.tasks_count * matrix_run_config.tasks_count;

  matrix_run_config.core = ::KrCoreInitialize(&CoreInitData);
  assert(matrix_run_config.core != nullptr);

  // Initialise delay Syncblock
  //
  KRCORE_SYNCBLOCK_INIT_DATA DelayInitData;
  ::memset(&DelayInitData, 0, sizeof(KRCORE_SYNCBLOCK_INIT_DATA));

  DelayInitData.ResetMode = CoreSyncblockResetModeManual;

  matrix_run_config.delay = ::KrCoreInitializeSyncblock(matrix_run_config.core, &DelayInitData);
  assert(matrix_run_config.delay != nullptr);

  // Initialise Heights
  //
  for (auto i = size_t(0); i < matrix.size(); ++i) {
    for (auto j = size_t(0); j < matrix.size(); ++j) {
      KRCORE_SYNCBLOCK_INIT_DATA SyncblockInitData;
      ::memset(&SyncblockInitData, 0, sizeof(KRCORE_SYNCBLOCK_INIT_DATA));

      SyncblockInitData.ResetMode = CoreSyncblockResetModeAutoAll;

      PKRCORE_SYNCBLOCK Syncblock = ::KrCoreInitializeSyncblock(matrix_run_config.core, &SyncblockInitData);
      assert(Syncblock != nullptr);

      matrix_run_config.heights.at(i, j) = size_t(0);
      matrix_run_config.heights_sync.at(i, j) = Syncblock;
    }
  }

  // Initialise Threads
  //
  for (auto i = size_t(0); i < matrix_run_config.threads_count; ++i) {
    KRCORE_THREAD_INIT_DATA ThreadInitData;
    ::memset(&ThreadInitData, 0, sizeof(KRCORE_THREAD_INIT_DATA));

    ThreadInitData.Binding.Type = SystemBindingLogicalProcessors;
    ThreadInitData.Binding.LogicalProcessors.Group = 0;
    ThreadInitData.Binding.LogicalProcessors.Mask  = 1ULL << i;

    matrix_run_config.threads[i] = ::KrCoreInitializeThread(matrix_run_config.core, &ThreadInitData);

    assert(matrix_run_config.threads[i] != nullptr);
  }

  // Initialise Tasks
  //
  for (auto i = size_t(0); i < matrix_run_config.tasks_count; ++i) {
    KRCORE_TASK_INIT_DATA TaskInitData;
    ::memset(&TaskInitData, 0, sizeof(KRCORE_TASK_INIT_DATA));

    KRCORE_TASK_ATTRIBUTE Attribute;
    ::memset(&Attribute, 0, sizeof(KRCORE_TASK_ATTRIBUTE));

    Attribute.Attribute = CoreTaskAttributeStartupCreateDelayed;

    if (i < matrix_run_config.threads_count)
      Attribute.ON_DELAY.Syncblock = matrix_run_config.delay;

    TaskInitData.AttributesCount = 1;
    TaskInitData.Attributes = &Attribute;
    TaskInitData.Binding.Type = SystemBindingLogicalProcessors;
    TaskInitData.Binding.LogicalProcessors.Group = 0;
    TaskInitData.Binding.LogicalProcessors.Mask  = 1ULL << (i % matrix_run_config.threads_count);
    TaskInitData.TaskRoutineState = &matrix_run_config.nodes[i];
    TaskInitData.TaskRoutine = calculation_routine;

    matrix_run_config.tasks[i] = ::KrCoreInitializeTask(matrix_run_config.core, &TaskInitData);

    assert(matrix_run_config.tasks[i] != nullptr);
  }
};

__hack_noinline
void
down(
  matrix_type&        matrix,
  matrix_access_type& matrix_access,
  matrix_run_config_type&  matrix_run_config,
  ::utilz::memory::buffer& b)
{
  for (auto i = size_t(0); i < matrix_run_config.tasks_count; ++i)
    KRASSERT(::KrCoreDeinitializeTask(matrix_run_config.core, matrix_run_config.tasks[i]));

  for (auto i = size_t(0); i < matrix_run_config.threads_count; ++i)
    KRASSERT(::KrCoreDeinitializeThread(matrix_run_config.core, matrix_run_config.threads[i]));

  for (auto i = size_t(0); i < matrix.size(); ++i) {
    for (auto j = size_t(0); j < matrix.size(); ++j) {
      KRASSERT(::KrCoreDeinitializeSyncblock(matrix_run_config.core, matrix_run_config.heights_sync.at(i, j)));
    }
  }

  KRASSERT(::KrCoreDeinitialize(matrix_run_config.core));
}

__hack_noinline
void
run(
  matrix_type& matrix,
  matrix_run_config_type& matrix_run_config)
{
  // Unblock all previously initialised Tasks
  //
  KRASSERT(::KrCoreSyncblockSignal(matrix_run_config.delay));

  // Wait them run to completion
  //
  for (auto i = size_t(0); i < matrix_run_config.tasks_count; ++i)
    ::WaitForSingleObject(matrix_run_config.tasks[i]->InternalThread, INFINITE);
};

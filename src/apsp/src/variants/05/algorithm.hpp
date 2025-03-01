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


// __forceinline
// void __fw_block(size_t block_size,
//                 long *ij_data,
//                 long *ik_data,
//                 long *kj_data) {
//     for (size_t k = 0ULL; k < block_size; ++k) {
// long *kj_row = &kj_data[k * block_size];
//         for (size_t i = 0ULL; i < block_size; ++i) {
//             long *ij_row = &ij_data[i * block_size];
//             long *__kj_row = kj_row;

//             long ik = ik_data[i * block_size + k];

//             #pragma vector aligned
//             #pragma vecremainder
//             for (size_t j = 0ULL; j < block_size; ++j, ++ij_row, ++__kj_row) {
//                 long distance = ik + *__kj_row;
//                 if (*ij_row > distance)
//                     *ij_row = distance;
//             };
//         };
//     };

//     #if defined(_DEBUG) || defined(_FW_STATISTICS)
//     __statistics_block_general_blocks_calculated++;
//     #endif
// };

// __forceinline
// void __fw_auto_block(size_t matrix_size,
//                      square_matrix<long, fw_numa_allocator<long>> *matrix_data,
//                      size_t block_size,
//                      size_t block_index,
//                      size_t i,
//                      size_t j) {
//     auto *ij = matrix_data[i * matrix_size + j].get_pointer();
//     if (block_index != i) {
//         if (block_index == j) {
//             auto *current = matrix_data[block_index * matrix_size + block_index].get_pointer();

//             __fw_block(block_size, ij, ij, current);

//             #if defined(_DEBUG) || defined(_FW_STATISTICS)
//             __statistics_stream_node_cross_blocks_calculated[i]++;
//             #endif
//         } else {
//             auto *west_or_east = matrix_data[i * matrix_size + block_index].get_pointer(),
//                  *north_or_south = matrix_data[block_index * matrix_size + j].get_pointer();

//             __fw_block(block_size, ij, west_or_east, north_or_south);

//             #if defined(_DEBUG) || defined(_FW_STATISTICS)
//             __statistics_stream_node_rest_blocks_calculated[i]++;
//             #endif
//         };
//     } else if (block_index != j) {
//         auto *current = matrix_data[block_index * matrix_size + block_index].get_pointer();

//         __fw_block(block_size, ij, current, ij);

//         #if defined(_DEBUG) || defined(_FW_STATISTICS)
//         __statistics_stream_node_cross_blocks_calculated[i]++;
//         #endif
//     } else {
//         __fw_block(block_size, ij, ij, ij);

//         #if defined(_DEBUG) || defined(_FW_STATISTICS)
//         __statistics_stream_node_diagonal_blocks_calculated[i]++;
//         #endif
//     };
//     #if defined(_DEBUG) || defined(_FW_STATISTICS)
//     __statistics_stream_node_blocks_calculated[i]++;
//     #endif
// };

// __declspec(noinline)
// void __fw_ak_kernel_wait_block(size_t matrix_size,
//                                short *matrix_data,
//            size_t rank,
//                                size_t block_index,
//                                size_t row,
//                                size_t col) {
// short *p_v = &matrix_data[row * matrix_size + col];

//     short v = *p_v, z = static_cast<short>(block_index + 1ULL);
//     #if defined(_DEBUG) || defined(_FW_STATISTICS)
//     if (v >= z) __statistics_stream_sync_fetch++;
//     #endif

//     while (v < z) {
//         #if defined(_DEBUG) || defined(_FW_STATISTICS)
//         measure statistics;
//         statistics.begin();
//         #endif
//         KRASSERT(::KrCoreTaskCurrentSynchronizeOnAddress(p_v, &v, sizeof(short)));

//         v = *p_v;

//         #if defined(_DEBUG) || defined(_FW_STATISTICS)
//         statistics.end();

// if (rank == 39) {
// rank = 39;
// }
// __statistics_stream_node_sync_wait_time[rank] += statistics.elapsed_milliseconds();

//         if (v < z) __statistics_stream_sync_miss++;
//         else __statistics_stream_sync_load++;
//         #endif
//     };
// };

// __declspec(noinline)
// void __fw_ak_kernel_notify_block(size_t matrix_size,
//                                  short *matrix_data,
//                                  size_t progress_value,
//                                  size_t row,
//                                  size_t col) {
//     short *p_v = &matrix_data[row * matrix_size + col];
//     *p_v = static_cast<short>(progress_value + 1ULL);

//     #if defined(_DEBUG) || defined(_FW_STATISTICS)
//     __statistics_stream_sync_store++;
//     #endif

//     KRASSERT(::KrManagerPulseAddress(p_v));
// };

// __declspec(noinline)
// void __fw_ak_kernel_switch(PKRCORE_TASK p_task) {
//     #if defined(_DEBUG) || defined(_FW_STATISTICS)

// {
// unsigned long core_number = ::GetCurrentProcessorNumber();

// std::unique_lock<std::mutex> lock(__statistics_stream_core_switch_mutex);
// __statistics_stream_core_switch[core_number]++;
// }

// __statistics_stream_switch++;
//     #endif

//     KRASSERT(::KrCoreTaskCurrentSwitchToTask(p_task));
// };

template<typename T, typename A>
__hack_noinline void
run(
  ::utilz::matrices::square_matrix<T, A>& m)
{
  // KRCORE_INIT_DATA CoreInitData = { 0 };
  // CoreInitData.TraceKey = 1UL;

  // PKRCORE Core = ::KrCoreInitialize(&CoreInitData);

  // using kr_task_state = std::tuple<std::vector<PKRCORE_TASK>*, std::vector<fw_stream_node>*, fw_stream_node*>;

  //               if (!::KrManagerInitialize())
  //                   throw std::runtime_error("cannot run kernel -> " + std::to_string(::GetLastError()));


  //               std::vector<std::string> error_messages;

  //               std::vector<PKRCORE_TASK> kr_task_index(stream_nodes.size());
  //               std::vector<kr_task_state> kr_task_state_object(stream_nodes.size());
  //               for (size_t i = 0ULL; i < kr_task_state_object.size(); ++i)
  //                   kr_task_state_object[i] = std::make_tuple(&kr_task_index, &stream_nodes, &stream_nodes[i]);

  //               size_t processor_count = stream_nodes[0].m_processors_count;
  //               for (size_t i = 0ULL; i < kr_task_state_object.size(); ++i) {
  //                   if (error_messages.empty()) {
  //                       KRCORE_TASK_INIT_DATA init_data = { 0 };
  //                       KRCORE_TASK_ATTRIBUTE attributes = {
  //                           KRCORE_TASK_ATTRIBUTES::CoreTaskAttributeStartupCreateDelayed
  //                       };

  //                       init_data.SYSTEM.dwTaskProcessor = static_cast<DWORD>(stream_nodes[i].m_processor);
  //                       init_data.SYSTEM.STARTUP.pTaskState = reinterpret_cast<void*>(&kr_task_state_object[i]);
  //                       init_data.SYSTEM.STARTUP.lpTaskStartRoutine = [](void* p_state) -> unsigned long {
  //                           auto *p_kr_task_state = reinterpret_cast<kr_task_state*>(p_state);

  //                           auto &kr_task_index = *std::get<0>(*p_kr_task_state);
  //                           auto &kr_stream_node_index = *std::get<1>(*p_kr_task_state);
  //                           auto &kr_stream_node = *std::get<2>(*p_kr_task_state);
  //                           auto &kr_stream = *kr_stream_node.mp_stream;

  //                           auto *block_matrix = kr_stream.m_block_matrix.get_pointer();
  //                           auto *block_progress_matrix = kr_stream.m_progress_matrix.get_pointer();

  //                           const size_t block_size = kr_stream.m_block_size;
  //                           const size_t block_count = kr_stream.m_block_count;

  //                           const size_t processor_count = kr_stream_node.m_processors_count;
  //                           const size_t processor = kr_stream_node.m_processor;
  //                           const size_t rank = kr_stream_node.m_rank;

  //                           const size_t kr_top_task = rank % processor_count;
  //                           const size_t kr_bottom_task = block_count - (processor_count - kr_top_task);
  //                           const size_t kr_previous_task = rank - processor_count;
  //                           const size_t kr_next_task = rank + processor_count;

  //                           const bool move_top = rank != kr_top_task;
  //                           const bool move_bottom = rank != kr_bottom_task;

  //                           size_t kr_src_task = kr_top_task;

  //                           bool is_leader = false;
  //                           bool is_follower = rank < processor_count;
  //                           bool is_normalized = false;

  //                           for (size_t j = 0ULL; j < block_count; ++j) {
  //                               if (rank <= j) {
  //                                   for (size_t block_index = 0ULL; block_index < rank; ++block_index) {
  //                                       __fw_ak_kernel_wait_block(block_count, block_progress_matrix, rank, block_index, block_index, j);

  //                                       __fw_auto_block(block_count, block_matrix, block_size, block_index, rank, j);
  //                                   };
  //                                   __fw_auto_block(block_count, block_matrix, block_size, rank, rank, j);

  //                                   __fw_ak_kernel_notify_block(block_count, block_progress_matrix, rank, rank, j);
  //                               } else {
  //                                   for (size_t block_index = 0ULL; block_index <= j; ++block_index) {
  //                                       __fw_ak_kernel_wait_block(block_count, block_progress_matrix, rank, block_index, block_index, j);

  //                                       __fw_auto_block(block_count, block_matrix, block_size, block_index, rank, j);
  //                                   };
  //                               };
  //                               if (!is_leader) {
  //                                   if (rank == j) {
  //                                       for (size_t reverse_j = 0ULL; reverse_j < j; ++reverse_j) {
  //                                           __fw_auto_block(block_count, block_matrix, block_size, rank, rank, reverse_j);

  //                                           __fw_ak_kernel_notify_block(block_count, block_progress_matrix, rank, rank, reverse_j);
  //                                       };

	// 									for (size_t forward_j = j + 1ULL; forward_j < block_count; ++forward_j) {
	// 										for (size_t block_index = 0ULL; block_index < rank; ++block_index) {
	// 											__fw_ak_kernel_wait_block(block_count, block_progress_matrix, rank, block_index, block_index, forward_j);

	// 											__fw_auto_block(block_count, block_matrix, block_size, block_index, rank, forward_j);
	// 										};
	// 										__fw_auto_block(block_count, block_matrix, block_size, rank, rank, forward_j);

	// 										__fw_ak_kernel_notify_block(block_count, block_progress_matrix, rank, rank, forward_j);
	// 									};

  //                                       if (move_top)
  //                                           __fw_ak_kernel_switch(kr_task_index[kr_top_task]);

  //                                       if (move_bottom)
  //                                           __fw_ak_kernel_switch(kr_task_index[kr_next_task]);

  //                                       is_leader = true;
  //                                       break;
  //                                   } else {
  //                                       if (!is_follower) {
  //                                           if (move_bottom) {
  //                                               __fw_ak_kernel_switch(kr_task_index[kr_next_task]);
  //                                           } else {
  //                                               __fw_ak_kernel_switch(kr_task_index[kr_src_task]);

  //                                               if (j == kr_src_task)
  //                                                   kr_src_task += processor_count;
  //                                           };
	// 										is_follower = is_follower || (j == kr_previous_task);
	// 										if (is_follower && !is_normalized) {
	// 											for (size_t reverse_j = 0ULL; reverse_j < j; ++reverse_j) {
	// 												for (size_t block_index = (reverse_j + 1UL); block_index <= j; ++block_index) {
	// 													__fw_auto_block(block_count, block_matrix, block_size, block_index, rank, reverse_j);
	// 												};
	// 											};
	// 											is_normalized = true;
	// 										};
  //                                       } else {
  //                                           if (move_top)
  //                                               __fw_ak_kernel_switch(kr_task_index[kr_top_task]);

  //                                           if (move_bottom)
  //                                               __fw_ak_kernel_switch(kr_task_index[kr_next_task]);

	// 										for (size_t reverse_j = 0ULL; reverse_j < j; ++reverse_j) {
	// 											__fw_ak_kernel_wait_block(block_count, block_progress_matrix, rank, j, j, reverse_j);

	// 											__fw_auto_block(block_count, block_matrix, block_size, j, rank, reverse_j);
	// 										};
  //                                       };

  //                                   };
  //                               };
  //                           };

  //                           is_leader = false;
  //                           is_follower = false;
  //                           if (move_bottom) {
  //                               __fw_ak_kernel_switch(kr_task_index[kr_next_task]);

  //                               for (size_t j = (rank + 1ULL); j <= kr_bottom_task; ++j) {
  //                                   for (size_t block_index = (rank + 1ULL); block_index <= j; ++block_index) {
  //                                       __fw_auto_block(block_count, block_matrix, block_size, block_index, rank, j);
	// 								};

  //                                   __fw_ak_kernel_switch(kr_task_index[kr_next_task]);
  //                               };

  //                               for (size_t reverse_j = 0ULL; reverse_j <= rank; ++reverse_j) {
	// 								for (size_t block_index = (rank + 1ULL); block_index <= kr_bottom_task; ++block_index) {
	// 									__fw_auto_block(block_count, block_matrix, block_size, block_index, rank, reverse_j);
	// 								};
  //                               };
  //                               for (size_t reverse_j = (rank + 1ULL); reverse_j < kr_bottom_task; ++reverse_j) {
	// 								for (size_t block_index = (reverse_j + 1ULL); block_index <= kr_bottom_task; ++block_index) {
	// 									__fw_auto_block(block_count, block_matrix, block_size, block_index, rank, reverse_j);
	// 								};
  //                               };
  //                               for (size_t reverse_j = (kr_bottom_task + 1ULL); reverse_j < block_count; ++reverse_j) {
	// 								for (size_t block_index = (rank + 1ULL); block_index <= kr_bottom_task; ++block_index) {
	// 									__fw_auto_block(block_count, block_matrix, block_size, block_index, rank, reverse_j);
	// 								};
  //                               };

  //                               __fw_ak_kernel_switch(kr_task_index[kr_next_task]);

  //                               for (size_t reverse_j = (kr_bottom_task + 1ULL); reverse_j < block_count; ++reverse_j) {
  //                                   __fw_auto_block(block_count, block_matrix, block_size, reverse_j, rank, reverse_j);

  //                                   __fw_ak_kernel_switch(kr_task_index[kr_next_task]);

	// 								for (size_t inner_j = 0ULL; inner_j < reverse_j; ++inner_j) {
	// 									__fw_auto_block(block_count, block_matrix, block_size, reverse_j, rank, inner_j);
	// 								};
	// 								for (size_t inner_j = (reverse_j + 1ULL); inner_j < block_count; ++inner_j) {
	// 									__fw_auto_block(block_count, block_matrix, block_size, reverse_j, rank, inner_j);
	// 								};

  //                                   __fw_ak_kernel_switch(kr_task_index[kr_next_task]);
  //                               };
  //                           } else {
  //                               __fw_ak_kernel_switch(kr_task_index[kr_top_task]);

  //                               for (size_t reverse_j = (kr_bottom_task + 1ULL); reverse_j < block_count; ++reverse_j) {
  //                                   __fw_ak_kernel_wait_block(block_count, block_progress_matrix, rank, reverse_j, reverse_j, reverse_j);

  //                                   __fw_auto_block(block_count, block_matrix, block_size, reverse_j, rank, reverse_j);

  //                                   __fw_ak_kernel_switch(kr_task_index[kr_top_task]);

  //                                   for (size_t inner_j = 0ULL; inner_j < reverse_j; ++inner_j) {
  //                                       __fw_ak_kernel_wait_block(block_count, block_progress_matrix, rank, reverse_j, reverse_j, inner_j);

  //                                       __fw_auto_block(block_count, block_matrix, block_size, reverse_j, rank, inner_j);
  //                                   };
  //                                   for (size_t inner_j = (reverse_j + 1ULL); inner_j < block_count; ++inner_j) {
  //                                       __fw_ak_kernel_wait_block(block_count, block_progress_matrix, rank, reverse_j, reverse_j, inner_j);

  //                                       __fw_auto_block(block_count, block_matrix, block_size, reverse_j, rank, inner_j);
  //                                   };

  //                                   __fw_ak_kernel_switch(kr_task_index[kr_top_task]);
  //                               };
  //                           };

  //                           if (rank == kr_bottom_task) {
  //                               for (size_t i = kr_top_task; i < kr_bottom_task; i += processor_count)
  //                                   KRASSERT(::KrCoreTaskContinue(kr_task_index[i]));
  //                           };
  //                           return 0UL;
  //                       };

  //                       if (stream_nodes[i].m_rank >= processor_count) {
  //                           init_data.CORE.ATTRIBUTES.dwCount = 1;
  //                           init_data.CORE.ATTRIBUTES.pAttributes = &attributes;
  //                       };

  //                       PKRCORE_TASK p_kr_core_task = ::KrManagerTaskInitialize(&init_data);
  //                       if (p_kr_core_task == nullptr) {
  //                           error_messages.push_back("cannot create kernel task -> " + std::to_string(::GetLastError()));

  //                           for (size_t j = 0ULL; j < i; ++j) {
  //                               if (!::TerminateThread(kr_task_index[j]->SYSTEM.hHandle, 1UL))
  //                                   error_messages.push_back("cannot terminate kernel task -> " + std::to_string(::GetLastError()));
  //                           };
  //                       };
  //                       kr_task_index[i] = p_kr_core_task;
  //                   };
  //               };

  //               if (error_messages.empty()) {
  //                   std::for_each(kr_task_index.rbegin(), kr_task_index.rend(), [&error_messages](PKRCORE_TASK p_kr_task) -> void {
  //                       if (!::KrCoreTaskSubmit(p_kr_task))
  //                           error_messages.push_back("cannot submit kernel task -> " + std::to_string(::GetLastError()));
  //                   });

  //                   if (error_messages.empty()) {
  //                       std::for_each(kr_task_index.rbegin(), kr_task_index.rend(), [](PKRCORE_TASK p_kr_task) -> void {
  //                           ::WaitForSingleObject(p_kr_task->SYSTEM.hHandle, INFINITE);
	// 					});

  //                       std::for_each(kr_task_index.rbegin(), kr_task_index.rend(), [&error_messages](PKRCORE_TASK p_kr_task) -> void {
  //                           if (!::KrManagerTaskDeinitialize(p_kr_task))
  //                               error_messages.push_back("cannot deinitialize kernel task -> " + std::to_string(::GetLastError()));
  //                       });
	// 				};
  //               };

  //               if (!::KrManagerDeinitialize())
  //                   throw std::runtime_error("cannot shutdown kernel -> " + std::to_string(::GetLastError()));

  //               if (!error_messages.empty()) {
  //                   std::stringstream errors;
  //                   std::for_each(error_messages.begin(), error_messages.end(), [&errors](std::string &error_message) -> void {
  //                       errors << error_message << std::endl;
  //                   });
  //                   throw std::runtime_error(errors.str());
  //               };
};

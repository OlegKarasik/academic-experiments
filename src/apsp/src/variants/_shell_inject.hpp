#pragma once

#include "memory.hpp"
#include "matrix.hpp"
#include "matrix-traits.hpp"
#include "matrix-access.hpp"
#include "matrix-manip.hpp"

// Define global types
//

using g_type = int;

template<typename T>
using g_allocator_type = typename ::utilz::memory::buffer_allocator<T>;

// Define namespaces
//
namespace utzmx = ::utilz::matrices;

// Inject the "algorithm"
//

#include "algorithm.hpp"

// Define values
//

using schema_value = matrix_access_type::schema_value;

// Define well-known and "algorithm" types
//

using size_type               = typename ::utilz::matrices::traits::matrix_traits<matrix_type>::size_type;
using value_type              = typename ::utilz::matrices::traits::matrix_traits<matrix_type>::value_type;
using graph_type              = typename std::tuple<size_type, std::vector<std::tuple<size_type, size_type, value_type>>>;
using communities_type        = typename std::map<size_type, std::vector<size_type>>;
using graph_format_type       = ::utilz::graphs::io::graph_format;
using communities_format_type = ::utilz::communities::io::communities_format;
using scan_matrix_params_type = ::utilz::matrices::io::scan_matrix_params<matrix_type>;

// Define procedures
//
using matrix_arrange_procedure_type = ::utilz::matrices::procedures::matrix_arrange_procedure<schema_value::value, matrix_type>;

// Define type stubs
//
#if not defined(APSP_ALG_MATRIX_CLUSTERS)
  using matrix_clusters_type = int;
#endif
#if not defined(APSP_ALG_RUN_CONFIGURATION)
  using matrix_run_config_type = int;
#endif

// Define methods
//

#ifdef APSP_ALG_MATRIX_FLAT
  #ifdef APSP_ALG_RUN_CONFIGURATION
    #define SHELL_RUN(matrix, matrix_clusters, matrix_run_config) (void)matrix_clusters; run(matrix, matrix_run_config)
  #else
    #define SHELL_RUN(matrix, matrix_clusters, matrix_run_config) (void)matrix_clusters; (void)matrix_run_config; run(matrix)
  #endif
#endif
#ifdef APSP_ALG_MATRIX_BLOCKS
  #ifdef APSP_ALG_RUN_CONFIGURATION
    #define SHELL_RUN(matrix, matrix_clusters, matrix_run_config) (void)matrix_clusters; run(matrix, matrix_run_config)
  #else
    #define SHELL_RUN(matrix, matrix_clusters, matrix_run_config) (void)matrix_clusters; (void)matrix_run_config; run(matrix)
  #endif
#endif
#ifdef APSP_ALG_MATRIX_CLUSTERS
  #ifdef APSP_ALG_RUN_CONFIGURATION
    #define SHELL_RUN(matrix, matrix_clusters, matrix_run_config) run(matrix, matrix_clusters, matrix_run_config)
  #else
    #define SHELL_RUN(matrix, matrix_clusters, matrix_run_config) (void)matrix_run_config; run(matrix, matrix_clusters)
  #endif
#endif

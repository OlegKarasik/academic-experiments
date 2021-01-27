#pragma once

#include <chrono>
#include <random>
#include <vector>

namespace utilz {
namespace graphs {

template<typename T, typename O>
void
random_weighted_directed_acyclic_graph(size_t v, size_t e, T min, T max, O out)
{
  std::tr1::mt19937_64 distribution_engine;

  std::tr1::uniform_int_distribution<size_t> v_distribution(0, (v - 1));
  std::tr1::uniform_int_distribution<long>   w_distribution(min, max);

  distribution_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  // Initialize vector of vertexes with vertex indexes
  // i.e. `vector[0]` represents vertex `0`, `vector[1]` vertex `1` and so on and so forth
  //
  std::vector<size_t> vertexes(v);
  std::generate(vertexes.begin(), vertexes.end(), [n = 0]() mutable { return n++; });

  // Initialize vector of edges between vertexes
  // i.e. vector contains true in `[i * v + j]` if there is an edge between
  // `i` and `j`
  //
  std::vector<bool> edges(v * v);
  std::fill(edges.begin(), edges.end(), false);

  // Initialize output
  //
  out.init(v, e);

  // Permutate vector of vertexes
  //
  for (size_t i = 0; i < (v - 1); ++i)
    std::swap(vertexes[v_distribution(distribution_engine)], vertexes[i]);

  // Pick two random vertexts indexes and create an edge between them.
  // Repeat until required number of edges.
  //
  for (size_t c = 0, i = 0, j = 0, attempt_count = 0; c < e;) {
    // Don't create self-cycles
    //
    if ((i = v_distribution(distribution_engine)) == (j = v_distribution(distribution_engine)))
      continue;

    i = vertexes[i];
    j = vertexes[j];

    // We should create edges directed from "lowel" vertexes to "higher" only
    // to ensure no cycles are created.
    //
    if (i > j)
      std::swap(i, j);

    if (!edges[i * v + j]) {
      // If output has no i -> j edge
      //
      out.set(i, j) = w_distribution(distribution_engine);
    } else if (attempt_count != e) {
      ++attempt_count;
      continue;
    } else {
      // If output has i -> j edge and we have tried to much to create
      // an edge (i.e. number of failed attempts is equal to required number of edges)
      // we simply perform a direct search to insert an edge
      //
      bool found = false;
      for (size_t _i = 0; _i < v && !found; ++_i)
        for (size_t _j = 0; _j < v && !found; ++_j) {
          i = vertexes[_i];
          j = vertexes[_j];

          if (i > j)
            std::swap(i, j);

          if (!edges[i * v + j]) {
            out.set(i, j) = w_distribution(distribution_engine);

            found = true;
          }
        }

      attempt_count = 0;
    }

    // Indicate that we have created an edge between `i` -> `j`
    //
    ++c;
    edges[i * v + j] = true;
  };
};

} // namespace graphs
} // namespace utilz

#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <deque>
#include <map>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

#include "square-shape.hpp"

namespace utilz {
namespace graphs {
namespace generators {

struct directed_acyclic_graph_tag
{
};

struct connected_graph_tag
{
};

struct complete_graph_tag
{
};

utilz::square_shape<bool>
random_graph(
  typename utilz::traits::square_shape_traits<square_shape<bool>>::size_type v,
  typename utilz::traits::square_shape_traits<square_shape<bool>>::size_type e,
  directed_acyclic_graph_tag)
{
  using size_type  = typename utilz::traits::square_shape_traits<square_shape<bool>>::size_type;

  static_assert(std::is_unsigned<size_type>::value, "erro: matrix `set_size` operation has to use unsigned integral type");

  if (e <= 0)
    throw std::logic_error(
      "erro: edge count in directed acyclic graph can't be zero or negative.");

  if (e >= (v * (v - size_type(1)) / size_type(2)))
    throw std::logic_error(
      "erro: edge count in directed acyclic graph can't exceed: `((v) * (v - 1) / 2)`, where `v` is a vertex count.");

  std::mt19937_64                          distribution_engine;
  std::uniform_int_distribution<size_type> distribution(size_type(0), v - size_type(1));

  distribution_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  // Initialize vector of vertexes with vertex indexes
  // i.e. `vector[0]` represents vertex `0`, `vector[1]` vertex `1` and so on and so forth
  //
  std::vector<size_type> vertexes(v);
  std::generate(vertexes.begin(), vertexes.end(), [n = size_type(0)]() mutable {
    return n++;
  });

  // Permutate vector of vertexes
  //
  for (size_type i = size_type(0); i < (v - size_type(1)); ++i)
    std::swap(vertexes[distribution(distribution_engine)], vertexes[i]);

  // Initialise adjacency matrix
  //
  utilz::square_shape<bool> adjacency_matrix(v);

  // Pick two random vertexts indexes and create an edge between them.
  // Repeat until required number of edges.
  //
  for (size_type i = size_type(0), j = size_type(0), c = size_type(0), a = size_type(0); c < e;) {
    // Don't create self-cycles
    //
    if ((i = distribution(distribution_engine)) == (j = distribution(distribution_engine)))
      continue;

    i = vertexes[i];
    j = vertexes[j];

    // We should create edges directed from "lowel" vertexes to "higher" only
    // to ensure no cycles are created.
    //
    if (i > j)
      std::swap(i, j);

    if (!adjacency_matrix.at(i, j)) {
      // If output has no i -> j edge
      //
      adjacency_matrix.at(i, j) = true;
    } else if (a != e) {
      ++a;
      continue;
    } else {
      // If output has i -> j edge and we have tried to much to create
      // an edge (i.e. number of failed attempts is equal to required number of edges)
      // we simply perform a direct search to insert an edge
      //
      bool found = false;
      for (size_type _i = size_type(0); _i < v && !found; ++_i)
        for (size_type _j = size_type(0); _j < v && !found; ++_j) {
          i = vertexes[_i];
          j = vertexes[_j];

          // Ensure we aren't creating self-cycles
          //
          if (i == j)
            continue;

          // Ensure we aren't creating bi-directed edges
          //
          if (i > j)
            std::swap(i, j);

          if (!adjacency_matrix.at(i, j)) {
            adjacency_matrix.at(i, j) = true;

            found = true;
          }
        }

      a = size_type(0);

      if (!found)
        throw std::logic_error(
          "erro: unable to create a edge between two vertexes without breaking the constrains, blame random or relax the parameters");
    }

    // Indicate that we have created an edge between `i` -> `j`
    //
    ++c;
  };

  return std::move(adjacency_matrix);
};

utilz::square_shape<bool>
random_graph(
  typename utilz::traits::square_shape_traits<square_shape<bool>>::size_type v,
  typename utilz::traits::square_shape_traits<square_shape<bool>>::size_type e,
  connected_graph_tag)
{
  using size_type  = typename utilz::traits::square_shape_traits<square_shape<bool>>::size_type;

  static_assert(std::is_unsigned<size_type>::value, "erro: matrix `set_size` operation has to use unsigned integral type");

  if (e <= 0)
    throw std::logic_error(
      "erro: edge count can't be zero or negative.");

  if (e >= (v * (v - size_type(1)) / size_type(2)))
    throw std::logic_error(
      "erro: edge count in connected graph can't exceed: `((v) * (v - 1) / 2)`, where `v` is a vertex count.");

  // Initialise vertex distribution
  //
  std::mt19937_64                          distribution_engine;
  std::uniform_int_distribution<size_type> distribution(size_type(0), v - size_type(1));

  distribution_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  // Initialize vector of vertexes with vertex indexes
  // i.e. `vector[0]` represents vertex `0`, `vector[1]` vertex `1` and so on and so forth
  //
  std::vector<size_type> vertexes(v);
  std::generate(vertexes.begin(), vertexes.end(), [n = size_type(0)]() mutable {
    return n++;
  });

  // Permutate vector of vertexes
  //
  for (auto i = size_type(0); i < (v - size_type(1)); ++i)
    std::swap(vertexes[distribution(distribution_engine)], vertexes[i]);

  // Initialise adjacency matrix
  //
  utilz::square_shape<bool> adjacency_matrix(v);

  // Connect all vertexes
  //
  for (auto i = size_type(1); i < v; ++i) {
    auto j = distribution(distribution_engine) % i;

    adjacency_matrix.at(vertexes[i], vertexes[j]) = true;
  };

  // Pick two random vertexts indexes and create an edge between them.
  // Repeat until required number of edges.
  //
  for (auto i = size_type(0), j = size_type(0), c = v - size_type(1), a = size_type(0), z = size_type(0); c < e;) {
    // Don't create self-cycles
    //
    if ((i = distribution(distribution_engine)) == (j = distribution(distribution_engine)))
      continue;

    i = vertexes[i];
    j = vertexes[j];

    if (!adjacency_matrix.at(i, j)) {
      // If there is no edges, we have to ensure, that introducing the edge won't introduce
      // a cycle in the graph
      //

      adjacency_matrix.at(i, j) = true;
    } else if (a != e) {
      ++a;
      continue;
    } else {
      // If output has i -> j edge and we have tried to much to create
      // an edge (i.e. number of failed attempts is equal to required number of edges)
      // we simply perform a direct search to insert an edge
      //
      bool found = false;
      for (auto _i = z; _i < v && !found; ++_i, ++z) {
        for (auto _j = size_type(0); _j < v && !found; ++_j) {
          i = vertexes[_i];
          j = vertexes[_j];

          // Ensure we aren't creating self-cycles
          //
          if (i == j)
            continue;

          if (!adjacency_matrix.at(i, j)) {
            adjacency_matrix.at(i, j) = true;

            found = true;
          }
        }
      }

      a = size_type(0);

      if (!found)
        throw std::logic_error(
          "erro: unable to create a edge between two vertexes without breaking the constrains, blame random or relax the parameters");
    }

    // Indicate that we have created an edge between `i` -> `j`
    //
    ++c;
  };

  return std::move(adjacency_matrix);
};

utilz::square_shape<bool>
random_graph(
  typename utilz::traits::square_shape_traits<square_shape<bool>>::size_type v,
  complete_graph_tag)
{
  using size_type  = typename utilz::traits::square_shape_traits<square_shape<bool>>::size_type;

  // Initialise adjacency matrix
  //
  utilz::square_shape<bool> adjacency_matrix(v);

  // Because we are generating complete graph and not just graph
  // with cycles and number of edges the logic behind algorithm
  // is straightforward - allow all edges except self-loops
  //
  for (auto i = size_type(0); i < v; ++i) {
    for (auto j = size_type(0); j < v; ++j) {
      if (i == j) {
        // We aren't generating self-loops in complete graph
        //
        adjacency_matrix.at(i, j) = false;
      } else {
        adjacency_matrix.at(i, j) = true;
      }
    }
  }

  return std::move(adjacency_matrix);
};

} // namespace generators
} // namespace graphs
} // namespace utilz

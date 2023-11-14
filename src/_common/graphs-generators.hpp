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

namespace utilz {
namespace graphs {
namespace generators {

struct directed_graph_tag
{};

struct directed_acyclic_graph_tag
{};

struct complete_graph_tag
{};

template<typename size_type>
struct promised_path
{
  const size_type f;
  const size_type t;
  const size_type h;

  promised_path(size_type f, size_type t, size_type h)
    : f(f)
    , t(t)
    , h(h)
  {
  }
};

struct graph_options
{
  bool is_acyclic;
  bool is_connected;
};

template<
  typename Matrix,
  typename MatrixSetSizeOperation,
  typename MatrixSetValueOperation>
void
random_graph(
  typename MatrixSetSizeOperation::result_type                              v,
  typename MatrixSetSizeOperation::result_type                              e,
  std::vector<promised_path<typename MatrixSetSizeOperation::result_type>>& p,
  Matrix&                                                                   m,
  MatrixSetSizeOperation&                                                   set_size,
  MatrixSetValueOperation&                                                  set_value,
  graph_options                                                             options)
{
  using size_type  = typename MatrixSetSizeOperation::result_type;
  using value_type = typename MatrixSetValueOperation::result_type;

  static_assert(std::is_unsigned<size_type>::value, "erro: matrix `set_size` operation has to use unsigned integral type");

  if (e <= 0)
    throw std::logic_error(
      "erro: edge count can't be zero or negative.");

  if (options.is_acyclic) {
    if (e >= (v * (v - size_type(1)) / size_type(2)))
      throw std::logic_error(
        "erro: edge count in directed acyclic graph can't exceed: `((v) * (v - 1) / 2)`, where `v` is a vertex count.");
  }

  // Initialise vertex distribution
  //
  std::mt19937_64                          distribution_engine;
  std::uniform_int_distribution<size_type> distribution(size_type(0), v - size_type(1));

  distribution_engine.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

  // Initialize vector of vertexes with vertex indexes
  // i.e. `vector[0]` represents vertex `0`, `vector[1]` vertex `1` and so on and so forth
  //
  std::vector<size_type> vertexes(v);
  std::generate(vertexes.begin(), vertexes.end(), [n = size_type(0)]() mutable { return n++; });

  // Initialize vector of edges between vertexes
  // i.e. vector contains true in `[i * v + j]` if there is an edge between
  // `i` and `j`
  //
  std::vector<bool> edges(v * v);
  std::fill(edges.begin(), edges.end(), false);

  // Initialize a vector of paths between vertexes
  // i.e. vector contains true in `[i * v + j]` if there is a path between
  // `i` and `j`
  //
  std::vector<bool> paths(v * v);
  std::fill(paths.begin(), paths.end(), false);

  std::vector<size_type> paths_from(v);
  std::vector<size_type> paths_to(v);

  // Permutate vector of vertexes
  //
  for (size_type i = size_type(0); i < (v - size_type(1)); ++i)
    std::swap(vertexes[distribution(distribution_engine)], vertexes[i]);

  // Set output matrix size and start writing edges
  //
  set_size(m, size_type(v));

  // Initialise common lambdas to be used across the algorithm
  //
  auto has_edge = [&edges, &paths, options, v](size_type i, size_type j) -> bool {
    if (edges[i * v + j])
      return true;

    if (options.is_acyclic) {
      // If we are working with acyclic graphs then we
      // has to ensure before creating `i` -> `j` edge that there is
      // no path from `j` to `i`
      //
      // So, we locate all paths from `j`.
      //
      // If there is a path from `j` to `i` we return `true` and saying that
      // there is an "edge" between them and we can't continue.
      //
      return paths[j * v + i];
    }
    return false;
  };
  auto reg_edge = [&edges, &paths, &paths_from, &paths_to, options, v](size_type i, size_type j) -> void {
    // Register an edge from `i` to `j`
    //
    edges[i * v + j] = true;

    if (options.is_acyclic || options.is_connected) {
      // If the graph is acyclic or commented we have to make sure that all paths are tracked
      //
      // That is why we get all paths of `j` and insert them to all paths of
      // all vertexes who has a path to `i` including `i` itself
      //

      size_type from_count = size_type(0),
                to_count   = size_type(0);

      paths[i * v + j] = true;

      #pragma omp parallel for shared(from_count, to_count)
      for (size_type x = size_type(0); x < v; ++x) {
        if (paths[x * v + i] && !paths[x * v + j]) {
          paths[x * v + j] = true;

          size_type idx;

          #pragma omp atomic capture
          idx = from_count++;

          paths_from[idx] = x;
        }
        if (paths[j * v + x] && !paths[i * v + x]) {
          paths[i * v + x] = true;

          size_type idx;

          #pragma omp atomic capture
          idx = to_count++;

          paths_to[idx] = x;
        }
      }
      for (size_type f = size_type(0); f < from_count; ++f) {
        size_type from = paths_from[f];

        for (size_type t = size_type(0); t < to_count; ++t) {
          size_type to = paths_to[t];

          paths[from * v + to] = true;
        }
      }
    }
  };

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

    if (!has_edge(i, j)) {
      // If there is no edges, we have to ensure, that introducing the edge won't introduce
      // a cycle in the graph
      //

      set_value(m, i, j, value_type(1));
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

          if (!has_edge(i, j)) {
            set_value(m, i, j, value_type(1));

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
    reg_edge(i, j);
  };

  // In case we need a connected graph we first need to make sure that
  // all subgraphs (if there are) are connected
  //
  if (options.is_connected) {
  }
};

template<
  typename Matrix,
  typename MatrixSetSizeOperation,
  typename MatrixSetValueOperation>
void
random_graph(
  typename MatrixSetSizeOperation::result_type                              v,
  typename MatrixSetSizeOperation::result_type                              e,
  std::vector<promised_path<typename MatrixSetSizeOperation::result_type>>& p,
  Matrix&                                                                   m,
  MatrixSetSizeOperation&                                                   set_size,
  MatrixSetValueOperation&                                                  set_value,
  directed_acyclic_graph_tag)
{
  using size_type  = typename MatrixSetSizeOperation::result_type;
  using value_type = typename MatrixSetValueOperation::result_type;

  static_assert(std::is_unsigned<size_type>::value, "erro: matrix `set_size` operation has to use unsigned integral type");

  // Verify size of promised paths not exceeds maximum number of edges
  //
  if (!p.empty()) {
    // We initialize it to number of promised path because
    // as part of promised path generation we exclude a direct path between
    // vertexes of promised path, we have to ensure we still have enough free space for edges
    //
    size_type h = size_type(p.size());
    for (size_t i = size_t(0); i < p.size(); ++i) {
      auto& path = p[i];
      if (path.f >= path.t)
        throw std::logic_error(
          "erro: promised paths from 'higher' vertexes to 'lower' is not supported");

      if (path.h >= path.t - path.f)
        throw std::logic_error(
          "erro: promised paths can't contain more hoops than natural distance between vertex indexes");

      h += path.h;
    }

    if (h > e)
      throw std::logic_error(
        "erro: hoops count of promised paths is greater than edges count.");
  }

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
  std::generate(vertexes.begin(), vertexes.end(), [n = size_type(0)]() mutable { return n++; });

  // Initialize vector of edges between vertexes
  // i.e. vector contains true in `[i * v + j]` if there is an edge between
  // `i` and `j`
  //
  std::vector<bool> edges(v * v);
  std::fill(edges.begin(), edges.end(), false);

  // Permutate vector of vertexes
  //
  for (size_type i = size_type(0); i < (v - size_type(1)); ++i)
    std::swap(vertexes[distribution(distribution_engine)], vertexes[i]);

  // Set output matrix size and start writing edges
  //
  set_size(m, size_type(v));

  for (size_t i = size_t(0); i < p.size(); ++i) {
    auto& path = p[i];

    // Initialize an empty vector of intermediate vertexes
    //
    std::vector<size_type> points;

    // Insert start of promised path
    //
    points.push_back(path.f);

    for (size_type j = size_type(0); j < path.h - size_type(1);) {
      auto z = vertexes[distribution(distribution_engine)];
      if (path.f < z && path.t > z && std::find(points.begin(), points.end(), z) == points.end()) {
        points.push_back(z);

        ++j;
      }
    }

    // Insert end of promised path
    //
    points.push_back(path.t);

    // Sort points within promised path to ensure
    // they are ordered from "lower" to "higher"
    //
    std::sort(points.begin(), points.end());

    auto it_from = points.begin(),
         it_to   = std::next(it_from);

    while (it_to != points.end()) {
      auto f = *it_from,
           t = *it_to;

      set_value(m, f, t, value_type(1));

      edges[f * v + t] = true;

      it_from = it_to;
      it_to   = std::next(it_to);
    }
    e -= path.h;

    // Ensure there won't be a direct path between vertexes
    // of promised path
    //
    edges[path.f * v + path.t] = true;
  }

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

    if (!edges[i * v + j]) {
      // If output has no i -> j edge
      //
      set_value(m, i, j, value_type(1));
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

          if (!edges[i * v + j]) {
            set_value(m, i, j, value_type(1));

            found = true;
          }
        }

      a = size_type(0);
    }

    // Indicate that we have created an edge between `i` -> `j`
    //
    ++c;
    edges[i * v + j] = true;
  };
};

template<
  typename Matrix,
  typename MatrixSetSizeOperation,
  typename MatrixSetValueOperation>
void
random_graph(
  typename MatrixSetSizeOperation::result_type v,
  Matrix&                                      m,
  MatrixSetSizeOperation&                      set_size,
  MatrixSetValueOperation&                     set_value,
  complete_graph_tag)
{
  using size_type  = typename MatrixSetSizeOperation::result_type;
  using value_type = typename MatrixSetValueOperation::result_type;

  static_assert(std::is_unsigned<size_type>::value, "erro: matrix `set_size` operation has to use unsigned integral type");

  // Set output matrix size and start writing edges
  //
  set_size(m, size_type(v));

  // Because we are generating complete graph and not just graph
  // with cycles and number of edges the logic behind algorithm
  // is straightforward - allow all edges except self-loops
  //
  for (auto i = size_type(0); i < v; ++i) {
    for (auto j = size_type(0); j < v; ++j) {
      if (i == j) {
        // We aren't generating self-loops in complete graph
        //
        set_value(m, i, j, value_type(0));
      } else {
        set_value(m, i, j, value_type(1));
      }
    }
  }
};

} // namespace generators
} // namespace graphs
} // namespace utilz

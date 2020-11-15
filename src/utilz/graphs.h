#pragma once

#include <algorithm>
#include <chrono>
#include <iterator>
#include <map>
#include <random>
#include <set>
#include <vector>

#include "matrix.h"

namespace math {
namespace utilz {
    static size_t _random_index(size_t max_index)
    {
        return static_cast<size_t>(::rand() % max_index);
    };

    static square_matrix<long> random_complete_graph(size_t vertex_count,
        long min_weight,
        long max_weight,
        long no_edge_value)
    {
        std::tr1::mt19937 engine;
        std::tr1::uniform_int_distribution<long> weight_distribution(min_weight, max_weight);

        std::chrono::high_resolution_clock::time_point time_seed = std::chrono::high_resolution_clock::now();
        engine.seed(static_cast<long>(time_seed.time_since_epoch().count()));

        std::vector<size_t> graph_vertexes(vertex_count);
        square_matrix<long> graph_matrix(vertex_count, no_edge_value);

        for (size_t i = 0ULL; i < vertex_count; ++i)
            graph_vertexes[i] = i;

        for (size_t i = 0ULL; i < (vertex_count - 1); ++i)
            std::swap(graph_vertexes[i + _random_index(vertex_count - i)], graph_vertexes[i]);

        for (size_t i = 0ULL; i < vertex_count; ++i) {
            for (size_t j = 0ULL; j < vertex_count; ++j)
                graph_matrix(graph_vertexes[i], graph_vertexes[j]) = weight_distribution(engine);
        };

        return graph_matrix;
    };

    // Directed acyclic graph algorithm was adopted from: http://condor.depaul.edu/rjohnson/source/graph_ge.c and extended to
    // create connected graph.

    static square_matrix<long> random_directed_acyclic_graph(size_t vertex_count,
        size_t edge_count,
        long min_weight,
        long max_weight,
        long no_edge_value)
    {
        std::tr1::mt19937 engine;
        std::tr1::uniform_int_distribution<size_t> index_distribution(0, (vertex_count - 1));
        std::tr1::uniform_int_distribution<long> weight_distribution(min_weight, max_weight);

        std::chrono::high_resolution_clock::time_point time_seed = std::chrono::high_resolution_clock::now();
        engine.seed(static_cast<long>(time_seed.time_since_epoch().count()));

        std::vector<size_t> graph_vertexes(vertex_count);
        square_matrix<long> graph_matrix(vertex_count, no_edge_value);

        for (size_t i = 0ULL; i < vertex_count; ++i)
            graph_vertexes[i] = i;

        for (size_t i = 0ULL; i < (vertex_count - 1); ++i)
            std::swap(graph_vertexes[i + _random_index(vertex_count - i)], graph_vertexes[i]);

        for (size_t current_edge_count = 0ULL, i = 0ULL, j = 0ULL, attempt_count = 0ULL; current_edge_count < edge_count;) {
            if ((i = index_distribution(engine)) == (j = index_distribution(engine)))
                continue;

            if (i > j)
                std::swap(i, j);

            i = graph_vertexes[i];
            j = graph_vertexes[j];
            if (graph_matrix(i, j) == no_edge_value) {
                graph_matrix(i, j) = weight_distribution(engine);
                current_edge_count++;

                attempt_count = 0ULL;
            } else if (++attempt_count == edge_count) {
                bool found = false;
                for (size_t _i = 0ULL; _i < vertex_count && !found; ++_i) {
                    for (size_t _j = 0ULL; _j < vertex_count && !found; ++_j) {
                        size_t __i = graph_vertexes[_i];
                        size_t __j = graph_vertexes[_j];

                        if (_i > _j)
                            std::swap(__i, __j);

                        if (graph_matrix(__i, __j) == no_edge_value) {
                            graph_matrix(__i, __j) = weight_distribution(engine);
                            current_edge_count++;

                            found = true;
                        };
                    };
                };

                attempt_count = 0ULL;
            };
        };

        for (size_t i = 0ULL; i < vertex_count; ++i)
            graph_matrix(i, i) = no_edge_value;

        std::vector<size_t> vertexes_roots;
        std::map<size_t, std::vector<size_t>> vertex_referenced_by, vertex_references;

        for (size_t i = 0ULL; i < vertex_count; ++i) {
            std::vector<size_t> referenced_by, references;
            for (size_t j = 0ULL; j < vertex_count; ++j) {
                if (graph_matrix(i, j) != no_edge_value)
                    references.push_back(j);

                if (graph_matrix(j, i) != no_edge_value)
                    referenced_by.push_back(j);
            };
            if (!referenced_by.size())
                vertexes_roots.push_back(i);

            vertex_referenced_by[i] = std::move(referenced_by);
            vertex_references[i] = std::move(references);
        };

        if (vertexes_roots.size() > 1) {
            std::map<size_t, std::set<size_t>> vertex_root_vertexes;
            for (size_t i = 0ULL; i < vertexes_roots.size(); ++i) {
                size_t vertex = vertexes_roots[i];
                std::set<size_t> vertex_set = { vertex };
                std::vector<size_t> vertex_to_scan(vertex_references[vertex]);

                std::copy(vertex_to_scan.begin(), vertex_to_scan.end(), std::inserter(vertex_set, vertex_set.end()));

                while (vertex_to_scan.size() > 0) {
                    for (size_t j = 0ULL; j < vertex_references[vertex_to_scan.front()].size(); ++j) {
                        if (vertex_set.insert(j).second)
                            vertex_to_scan.push_back(j);
                    };
                    vertex_to_scan.erase(vertex_to_scan.begin());
                };
                vertex_root_vertexes[vertexes_roots[i]] = std::move(vertex_set);
            };

            size_t i, j, i_root, j_root;
            while (vertexes_roots.size() > 1) {
                if (vertexes_roots.size() == 2) {
                    i = 0;
                    j = 1;
                } else {
                    if ((i = _random_index(vertexes_roots.size() - 1)) == (j = _random_index(vertexes_roots.size() - 1)))
                        continue;
                };

                i = i_root = vertexes_roots[i];
                j = j_root = vertexes_roots[j];
                std::set<size_t>& i_vertexes = vertex_root_vertexes[i];
                std::set<size_t>& j_vertexes = vertex_root_vertexes[j];

                std::set<size_t> intersection;
                std::set_intersection(
                    i_vertexes.begin(),
                    i_vertexes.end(),
                    j_vertexes.begin(),
                    j_vertexes.end(),
                    std::inserter(intersection, intersection.end()));

                if (intersection.empty()) {
                    if (i_vertexes.size() > 1) {
                        auto it = i_vertexes.begin();
                        std::advance(it, _random_index(i_vertexes.size() - 1));

                        i = (*it);
                    };
                    if (j_vertexes.size() > 1) {
                        auto it = j_vertexes.begin();
                        std::advance(it, _random_index(j_vertexes.size() - 1));

                        j = (*it);
                    };
                    if (i > j) {
                        std::swap(i, j);
                        std::swap(i_root, j_root);
                        std::swap(i_vertexes, j_vertexes);
                    };
                    graph_matrix(i, j) = weight_distribution(engine);

                    std::copy(j_vertexes.begin(), j_vertexes.end(), std::inserter(i_vertexes, i_vertexes.end()));
                };
                vertexes_roots.erase(std::find(vertexes_roots.begin(), vertexes_roots.end(), j_root));
            };
        };
        return graph_matrix;
    };
};
};

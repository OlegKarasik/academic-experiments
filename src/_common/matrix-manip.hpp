#pragma once

#include <map>
#include <numeric>

#include "matrix.hpp"
#include "matrix-traits.hpp"
#include "matrix-access.hpp"

namespace utilz {
namespace matrices {
namespace procedures {

enum matrix_arrangement
{
  matrix_arrangement_forward  = 1,
  matrix_arrangement_backward = 2
};

template<access::matrix_access_schema TSchema, typename S>
struct matrix_arrange_procedure
{
private:
  using size_type  = typename traits::matrix_traits<S>::size_type;
  using value_type = typename traits::matrix_traits<S>::value_type;

private:
  template<typename Iterator>
  void
  arrange(access::matrix_access<TSchema, S>& matrix_access, Iterator begin, Iterator end)
  {
    auto dimensions = matrix_access.dimensions();

    for (auto it = begin; it != end; ++it) {
      if (it->first == it->second)
        continue;

      for (auto j = size_type(0); j < dimensions.w(); ++j)
        std::swap(matrix_access.at(it->first, j), matrix_access.at(it->second, j));

      for (auto i = size_type(0); i < dimensions.h(); ++i)
        std::swap(matrix_access.at(i, it->first), matrix_access.at(i, it->second));
    }
  }

public:
  void
  operator()(
    access::matrix_access<TSchema, S>& matrix_access,
    utilz::matrices::clusters& matrix_clusters,
    const matrix_arrangement arrangement)
  {
    std::map<size_type, size_type> mapping;

    auto mapping_idx = size_type(0);
    for (auto& group : matrix_clusters.list()) {
      for (auto& vertex : group.list()) {
        auto vertex_it = mapping.find(std::get<size_t>(vertex));
        if (vertex_it != mapping.end()) {
          auto lookup_vertex_it = vertex_it;
          while (true) {
            auto it = mapping.find(lookup_vertex_it->second);
            if (it != mapping.end()) {
              lookup_vertex_it = it;
            } else {
              vertex_it = lookup_vertex_it;
              break;
            }
          }
        }

        mapping.emplace(
          mapping_idx,
          vertex_it == mapping.end()
            ? std::get<size_t>(vertex)
            : vertex_it->second);

        ++mapping_idx;
      }
    }

    switch (arrangement) {
      case matrix_arrangement_forward:
        arrange(matrix_access, mapping.begin(), mapping.end());
        break;
      case matrix_arrangement_backward:
        arrange(matrix_access, mapping.rbegin(), mapping.rend());
        break;
      default:
        throw std::logic_error("erro: unsupported matrix clusters arrangement");
    }
  }
};

} // namespace procedures
} // namespace matrices
} // namespace utilz

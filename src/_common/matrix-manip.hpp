#pragma once

#include <map>
#include <numeric>

#include "matrix-traits.hpp"
#include "matrix.hpp"

namespace utilz {
namespace matrices {
namespace procedures {

enum matrix_clusters_arrangement
{
  matrix_clusters_arrangement_forward  = 1,
  matrix_clusters_arrangement_backward = 2
};

// ---
// Forward declarations
//

namespace impl {

template<typename S>
struct impl_set_all;

template<typename S>
struct impl_set_diagonal;

template<typename S>
struct impl_arrange;

} // namespace impl

//
// Forward declarations
// ---

template<typename S>
using abstract_set_all = impl::impl_set_all<S>;

template<typename S>
using abstract_set_diagonal = impl::impl_set_diagonal<S>;

template<typename S>
using abstract_arrange = impl::impl_arrange<S>;

namespace impl {

template<typename S>
struct impl_set_all
{
private:
  using size_type  = typename S::size_type;
  using value_type = typename S::value_type;

public:
  void
  operator()(S& s, value_type v)
  {
    for (auto i = size_type(0); i < s.size(); ++i)
      for (auto j = size_type(0); j < s.size(); ++j)
        s.at(i, j) = v;
  }
};

template<typename S>
struct impl_set_diagonal
{
private:
  using size_type  = typename S::size_type;
  using value_type = typename S::value_type;

public:
  void
  operator()(S& s, value_type v)
  {
    for (auto i = size_type(0); i < s.size(); ++i)
      s.at(i, i) = v;
  }
};

template<typename S>
struct impl_arrange
{
private:
  using size_type  = typename S::size_type;
  using value_type = typename S::value_type;

private:
  template<typename Iterator>
  void
  abstract_arrange(S& s, Iterator begin, Iterator end)
  {
    for (auto it = begin; it != end; ++it) {
      if (it->first == it->second)
        continue;

      for (auto j = size_type(0); j < s.size(); ++j)
        std::swap(s.at(it->first, j), s.at(it->second, j));

      for (auto i = size_type(0); i < s.size(); ++i)
        std::swap(s.at(i, it->first), s.at(i, it->second));
    }
  }

public:
  void
  operator()(S& s, utilz::matrices::clusters& clusters, const matrix_clusters_arrangement arrangement)
  {
    std::map<size_type, size_type> mapping;

    auto mapping_idx = size_type(0);
    for (auto& group : clusters.list()) {
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
      case matrix_clusters_arrangement_forward:
        abstract_arrange(s, mapping.begin(), mapping.end());
        break;
      case matrix_clusters_arrangement_backward:
        abstract_arrange(s, mapping.rbegin(), mapping.rend());
        break;
      default:
        throw std::logic_error("erro: unsupported matrix clusters arrangement");
    }
  }
};

} // namespace get_dimensions

} // namespace procedures
} // namespace matrices
} // namespace utilz

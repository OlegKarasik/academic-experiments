#pragma once

#include <map>
#include <ranges>
#include <vector>

#include <memory>
#include <utility>

namespace utilz {
namespace matrices {

enum clusters_vertex_flag
{
  clusters_vertex_flag_none         = 0,
  clusters_vertex_flag_input        = 1,
  clusters_vertex_flag_output       = 2,
  clusters_vertex_flag_input_output = 3
};
inline clusters_vertex_flag operator & (clusters_vertex_flag a, clusters_vertex_flag b)
{
  return static_cast<clusters_vertex_flag>(static_cast<int>(a) & static_cast<int>(b));
}
inline clusters_vertex_flag operator | (clusters_vertex_flag a, clusters_vertex_flag b)
{
  return static_cast<clusters_vertex_flag>(static_cast<int>(a) | static_cast<int>(b));
}

// ---
// Forward declarations
//

class group;
class clusters;

template<typename T, typename A>
class rect_matrix;

template<typename T, typename A>
class square_matrix;

//
// Forward declarations
// ---

class group
{
public:
  using index_t = size_t;

private:
  std::vector<std::tuple<index_t, clusters_vertex_flag>> m_vertices;

private:
  auto
  find(const index_t& vertex)
  {
    return std::ranges::find_if(
      this->m_vertices,
      [&vertex](auto v) -> bool { return std::get<index_t>(v) == vertex; });
  }

public:
  void
  insert(const index_t& vertex)
  {
    if (const auto it = this->find(vertex); it != this->m_vertices.end()) {
      throw std::logic_error("erro: Unable to insert duplicate vertex into the group");
    }
    this->m_vertices.emplace_back(vertex, clusters_vertex_flag_none);
  }

  void
  update(const index_t& vertex, const clusters_vertex_flag flag)
  {
    const auto it = this->find(vertex);
    if (it == this->m_vertices.end()) {
      throw std::logic_error("erro: Unable to update vertex which isn't included into the group");
    }
    *it = std::make_tuple(vertex, std::get<clusters_vertex_flag>(*it) | flag);
  }

  void
  sort(const std::array<clusters_vertex_flag, 3> arrangements)
  {
    std::array<long, 4> orders = { 3, 3, 3, 3 };
    orders[arrangements[0]] = 0;
    orders[arrangements[1]] = 1;
    orders[arrangements[2]] = 2;

    if (   arrangements[2] == clusters_vertex_flag_input
        || arrangements[2] == clusters_vertex_flag_output) {
      orders[clusters_vertex_flag_input_output] = 2;
    }
    if (   arrangements[1] == clusters_vertex_flag_input
        || arrangements[1] == clusters_vertex_flag_output) {
      orders[clusters_vertex_flag_input_output] = 1;
    }
    if (   arrangements[0] == clusters_vertex_flag_input
        || arrangements[0] == clusters_vertex_flag_output) {
      orders[clusters_vertex_flag_input_output] = 0;
    }

    std::ranges::sort(
      this->m_vertices,
      [&orders](auto a, auto b) -> bool {
        return std::tie(orders[std::get<clusters_vertex_flag>(a)], std::get<index_t>(a))
             < std::tie(orders[std::get<clusters_vertex_flag>(b)], std::get<index_t>(b));
      });
  }

  [[nodiscard]]
  size_t
  size() const
  {
    return this->m_vertices.size();
  }

  [[nodiscard]]
  auto
  list()
  {
    return std::views::all(this->m_vertices);
  }

  bool
  contains(const index_t& vertex)
  {
    return this->find(vertex) != this->m_vertices.end();
  }
};

class clusters
{
public:
  using index_t = size_t;

private:
  std::map<index_t, group>   m_groups;
  std::map<index_t, index_t> m_groups_lookup;

  std::unordered_map<index_t, std::vector<index_t>> m_bridges;
  std::unordered_map<index_t, std::vector<index_t>> m_bridges_input;
  std::unordered_map<index_t, std::vector<index_t>> m_bridges_output;
  std::unordered_map<index_t, std::vector<index_t>> m_bridges_position;
  std::unordered_map<index_t, std::vector<index_t>> m_bridges_positions_input;
  std::unordered_map<index_t, std::vector<index_t>> m_bridges_positions_output;

public:
  void
  insert_vertex(const index_t& group_idx, const index_t& vertex_idx)
  {
    auto it = this->m_groups.find(group_idx);
    if (it == this->m_groups.end()) {
      it = this->m_groups.emplace(group_idx, group()).first;
    }
    it->second.insert(vertex_idx);

    this->m_groups_lookup[vertex_idx] = group_idx;
  }

  void
  insert_edge(const index_t& from_idx, const index_t& to_idx)
  {
    const auto x = this->m_groups.find(this->m_groups_lookup.at(from_idx));
    const auto z = this->m_groups.find(this->m_groups_lookup.at(to_idx));

    if (!x->second.contains(to_idx)) {
      x->second.update(from_idx, clusters_vertex_flag_output);
    }
    if (!z->second.contains(from_idx)) {
      z->second.update(to_idx, clusters_vertex_flag_input);
    }
  }

  void
  optimise() {
    this->m_bridges.clear();
    this->m_bridges_input.clear();
    this->m_bridges_output.clear();

    this->m_bridges_position.clear();
    this->m_bridges_positions_input.clear();
    this->m_bridges_positions_output.clear();

    for (auto [key, group] : this->m_groups) {
      std::vector<index_t> bridges;
      std::vector<index_t> bridges_input;
      std::vector<index_t> bridges_output;

      std::vector<index_t> bridges_positions;
      std::vector<index_t> bridges_positions_input;
      std::vector<index_t> bridges_positions_output;

      auto i = static_cast<index_t>(0);
      for (auto& v : group.list()) {
        const auto position = i++;

        if (std::get<clusters_vertex_flag>(v) == clusters_vertex_flag_none) {
          continue;
        }

        const auto index    = std::get<index_t>(v);
        const auto flag     = std::get<clusters_vertex_flag>(v);

        bridges.emplace_back(index);
        bridges_positions.emplace_back(position);

        if (flag & clusters_vertex_flag_input) {
          bridges_input.emplace_back(index);
          bridges_positions_input.emplace_back(position);
        }
        if (flag & clusters_vertex_flag_output) {
          bridges_output.emplace_back(index);
          bridges_positions_output.emplace_back(position);
        }
      }

      this->m_bridges.emplace(key, bridges);
      this->m_bridges_input.emplace(key, bridges_input);
      this->m_bridges_output.emplace(key, bridges_output);

      this->m_bridges_position.emplace(key, bridges_positions);
      this->m_bridges_positions_input.emplace(key, bridges_positions_input);
      this->m_bridges_positions_output.emplace(key, bridges_positions_output);
    }
  }

  size_t
  size() const noexcept
  {
    return this->m_groups.size();
  }

  auto
  list()
  {
    return std::views::values(this->m_groups);
  }

  auto
  get_all_bridges(const index_t& group_idx) const
  {
    return std::views::all(this->m_bridges.at(group_idx));
  }

  auto
  get_input_bridges(const index_t& group_idx) const
  {
    return std::views::all(this->m_bridges_input.at(group_idx));
  }

  auto
  get_output_bridges(const index_t& group_idx) const
  {
    return std::views::all(this->m_bridges_output.at(group_idx));
  }

  auto
  get_all_bridges_positions(const index_t& group_idx) const
  {
    return std::views::all(this->m_bridges_position.at(group_idx));
  }

  auto
  get_input_bridges_positions(const index_t& group_idx) const
  {
    return std::views::all(this->m_bridges_positions_input.at(group_idx));
  }

  auto
  get_output_bridges_positions(const index_t& group_idx) const
  {
    return std::views::all(this->m_bridges_positions_output.at(group_idx));
  }
};

template<typename T, typename A = std::allocator<T>>
class rect_matrix
{
public:
  using value_type      = T;
  using size_type       = size_t;
  using allocator_type  = A;
  using reference       = T&;
  using const_reference = const T&;
  using pointer         = typename std::allocator_traits<allocator_type>::pointer;
  using const_pointer   = typename std::allocator_traits<allocator_type>::const_pointer;

private:
  allocator_type m_a;

  size_type m_width;
  size_type m_height;

  size_type m_msize;
  pointer   m_m;

  void
  allocate_resources()
  {
    this->m_m = std::allocator_traits<allocator_type>::allocate(this->m_a, this->m_msize);
  }
  void
  construct_default()
  {
    pointer   t = this->m_m;
    size_type n = this->m_msize;

    for (auto i = size_type(0); i < n; ++i)
      std::allocator_traits<allocator_type>::construct(this->m_a, t++);
  }
  void
  copy_insert_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = 0; i < n; ++i)
      std::allocator_traits<allocator_type>::construct(this->m_a, t++, *(f++));
  }
  void
  copy_assign_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = 0; i < n; ++i)
      *(t++) = *(f++);
  }
  void
  move_insert_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = 0; i < n; ++i)
      std::allocator_traits<allocator_type>::construct(this->m_a, t++, std::move(*(f++)));
  }
  void
  move_assign_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = size_type(0); i < n; ++i)
      *(t++) = std::move(*(f++));
  }
  void
  destroy_resources()
  {
    pointer   p = this->m_m;
    size_type n = this->m_msize;

    for (auto i = size_type(0); i < n; ++i)
      std::allocator_traits<A>::destroy(this->m_a, p++);
  }
  void
  free_resources()
  {
    if (this->m_msize > 0)
      std::allocator_traits<allocator_type>::deallocate(this->m_a, this->m_m, this->m_msize);
  }

public:
  rect_matrix()
    : m_a(allocator_type())
    , m_width(0)
    , m_height(0)
    , m_msize(0)
    , m_m(nullptr)
  {
  }
  rect_matrix(const allocator_type& a)
    : m_a(a)
    , m_width(0)
    , m_height(0)
    , m_msize(0)
    , m_m(nullptr)
  {
  }

  rect_matrix(size_type w, size_type h, const allocator_type& a)
    : m_a(a)
    , m_width(w)
    , m_height(h)
    , m_msize(w * h)
    , m_m(nullptr)
  {
    if ((w * h) > 0) {
      this->allocate_resources();
      this->construct_default();
    }
  }

  rect_matrix(const rect_matrix& o)
    : m_a(std::allocator_traits<allocator_type>::select_on_container_copy_construction(o.m_a))
    , m_width(o.m_width)
    , m_height(o.m_height)
    , m_msize(o.m_msize)
    , m_m(nullptr)
  {
    this->allocate_resources();
    this->copy_insert_resources_n(o.m_m, o.m_msize);
  }
  rect_matrix(rect_matrix&& o) noexcept
    : m_a(std::move(o.m_a))
    , m_width(std::exchange(o.m_width, 0))
    , m_height(std::exchange(o.m_height, 0))
    , m_msize(std::exchange(o.m_msize, 0))
    , m_m(std::move(o.m_m))
  {
  }
  rect_matrix(rect_matrix&& o, const allocator_type& a)
    : m_a(a)
    , m_width(o.m_width)
    , m_height(o.m_height)
    , m_msize(o.m_msize)
    , m_m(nullptr)
  {
    if (this->m_msize > 0) {
      if (this->m_a == o.m_a) {
        this->m_m = std::move(o.m_m);
      } else {
        this->allocate_resources();
        this->move_insert_resources_n(o.m_m, o.m_msize);

        // Release resources and reset members to ensure
        // no object will be 'destroyed' in 'destructor'
        //
        o.free_resources();
      }

      o.m_msize  = size_type();
      o.m_width  = size_type();
      o.m_height = size_type();
      o.m_m      = nullptr;
    }
  };

  allocator_type
  get_allocator() const noexcept
  {
    return this->m_a;
  };

  bool
  empty() const
  {
    return this->m_msize == 0;
  };

  size_type
  width() const
  {
    return this->m_width;
  };

  size_type
  height() const
  {
    return this->m_height;
  };

  pointer
  at(size_type i) noexcept
  {
    return &this->m_m[i * this->m_width];
  };
  const_pointer
  at(size_type i) const noexcept
  {
    return &this->m_m[i * this->m_width];
  };

  reference
  at(size_type i, size_type j) noexcept
  {
    return this->m_m[i * this->m_width + j];
  };
  const_reference
  at(size_type i, size_type j) const noexcept
  {
    return this->m_m[i * this->m_width + j];
  };

  bool
  operator==(const rect_matrix& o) const noexcept
  {
    return this == &o || (this->m_msize == o.m_msize && std::equal(this->m_m, this->m_m + this->m_msize, o.m_m));
  };
  bool
  operator!=(const rect_matrix& o) const noexcept
  {
    return !(*this == o);
  };

  rect_matrix&
  operator=(const rect_matrix& o)
  {
    if (this != &o) {
      this->destroy_resources();
      this->free_resources();

      if (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value)
        this->m_a = o.m_a;

      this->m_msize  = o.m_msize;
      this->m_width  = o.m_width;
      this->m_height = o.m_height;

      this->allocate_resources();
      this->copy_assign_resources_n(o.m_m, o.m_msize);
    }
    return *this;
  };
  rect_matrix&
  operator=(rect_matrix&& o) noexcept(
    std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value&& std::is_nothrow_move_assignable<allocator_type>::value)
  {
    if (this != &o) {
      if (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
        this->destroy_resources();
        this->free_resources();

        this->m_a      = std::move(o.m_a);
        this->m_m      = std::exchange(o.m_m, nullptr);
        this->m_width  = std::exchange(o.m_width, size_type());
        this->m_height = std::exchange(o.m_height, size_type());
        this->m_msize  = std::exchange(o.m_msize, size_type());
      } else {
        if (this->m_a == o.m_a) {
          this->destroy_resources();
          this->free_resources();

          this->m_m      = std::exchange(o.m_m, nullptr);
          this->m_width  = std::exchange(o.m_width, size_type());
          this->m_height = std::exchange(o.m_height, size_type());
          this->m_msize  = std::exchange(o.m_msize, size_type());
        } else {
          if (this->m_msize != o.m_msize) {
            this->destroy_resources();
            this->free_resources();
            this->allocate_resources();
          }
          this->move_assign_resources_n(o.m_m, o.m_msize);
        };
      };
    };

    return *this;
  };
};

template<typename T, typename A = std::allocator<T>>
class square_matrix
{
public:
  using value_type      = T;
  using size_type       = size_t;
  using allocator_type  = A;
  using reference       = T&;
  using const_reference = const T&;
  using pointer         = typename std::allocator_traits<allocator_type>::pointer;
  using const_pointer   = typename std::allocator_traits<allocator_type>::const_pointer;

private:
  allocator_type m_a;

  size_type m_size;

  size_type m_msize;
  pointer   m_m;

  void
  allocate_resources()
  {
    this->m_m = std::allocator_traits<allocator_type>::allocate(this->m_a, this->m_msize);
  }
  void
  construct_default()
  {
    pointer   t = this->m_m;
    size_type n = this->m_msize;

    for (auto i = size_type(0); i < n; ++i)
      std::allocator_traits<allocator_type>::construct(this->m_a, t++);
  }
  void
  copy_insert_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = size_type(0); i < n; ++i)
      std::allocator_traits<allocator_type>::construct(this->m_a, t++, *(f++));
  }
  void
  copy_assign_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = size_type(0); i < n; ++i)
      *(t++) = *(f++);
  }
  void
  move_insert_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = size_type(0); i < n; ++i)
      std::allocator_traits<allocator_type>::construct(this->m_a, t++, std::move(*(f++)));
  }
  void
  move_assign_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = size_type(0); i < n; ++i)
      *(t++) = std::move(*(f++));
  }
  void
  destroy_resources()
  {
    pointer   p = this->m_m;
    size_type n = this->m_msize;

    for (auto i = size_type(0); i < n; ++i)
      std::allocator_traits<A>::destroy(this->m_a, p++);
  }
  void
  free_resources()
  {
    if (this->m_msize > size_type(0))
      std::allocator_traits<allocator_type>::deallocate(this->m_a, this->m_m, this->m_msize);
  }

public:
  square_matrix()
    : square_matrix(allocator_type())
  {
  }
  square_matrix(const allocator_type& a)
    : m_a(a)
    , m_size(0)
    , m_msize(0)
    , m_m(nullptr)
  {
  }

  square_matrix(size_type s)
    : square_matrix(s, allocator_type())
  {
  }
  square_matrix(size_type s, const allocator_type& a)
    : m_a(a)
    , m_size(s)
    , m_msize(s * s)
    , m_m(nullptr)
  {
    if (s > 0) {
      this->allocate_resources();
      this->construct_default();
    }
  }

  square_matrix(const square_matrix& o)
    : m_a(std::allocator_traits<allocator_type>::select_on_container_copy_construction(o.m_a))
    , m_size(o.m_size)
    , m_msize(o.m_msize)
    , m_m(nullptr)
  {
    this->allocate_resources();
    this->copy_insert_resources_n(o.m_m, o.m_msize);
  }
  square_matrix(square_matrix&& o) noexcept
    : m_a(std::move(o.m_a))
    , m_size(std::exchange(o.m_size, 0))
    , m_msize(std::exchange(o.m_msize, 0))
    , m_m(std::move(o.m_m))
  {
  }
  square_matrix(square_matrix&& o, const allocator_type& a)
    : m_a(a)
    , m_size(o.m_size)
    , m_msize(o.m_msize)
    , m_m(nullptr)
  {
    if (this->m_msize > 0) {
      if (this->m_a == o.m_a) {
        this->m_m = std::move(o.m_m);
      } else {
        this->allocate_resources();
        this->move_insert_resources_n(o.m_m, o.m_msize);

        // Release resources and reset members to ensure
        // no object will be 'destroyed' in 'destructor'
        //
        o.free_resources();
      }

      o.m_msize = size_type();
      o.m_size  = size_type();
      o.m_m     = nullptr;
    }
  };

  allocator_type
  get_allocator() const noexcept
  {
    return this->m_a;
  };

  bool
  empty() const
  {
    return this->m_msize == 0;
  };

  size_type
  size() const
  {
    return this->m_size;
  };

  pointer
  at(size_type i) noexcept
  {
    return &this->m_m[i * this->m_size];
  };
  const_pointer
  at(size_type i) const noexcept
  {
    return &this->m_m[i * this->m_size];
  };

  reference
  at(size_type i, size_type j) noexcept
  {
    return this->m_m[i * this->m_size + j];
  };
  const_reference
  at(size_type i, size_type j) const noexcept
  {
    return this->m_m[i * this->m_size + j];
  };

  bool
  operator==(const square_matrix& o) const noexcept
  {
    return this == &o || (this->m_msize == o.m_msize && std::equal(this->m_m, this->m_m + this->m_msize, o.m_m));
  };
  bool
  operator!=(const square_matrix& o) const noexcept
  {
    return !(*this == o);
  };

  square_matrix&
  operator=(const square_matrix& o)
  {
    if (this != &o) {
      this->destroy_resources();
      this->free_resources();

      if (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value)
        this->m_a = o.m_a;

      this->m_msize = o.m_msize;
      this->m_size  = o.m_size;

      this->allocate_resources();
      this->copy_assign_resources_n(o.m_m, o.m_msize);
    }
    return *this;
  };
  square_matrix&
  operator=(square_matrix&& o) noexcept(
    std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value&& std::is_nothrow_move_assignable<allocator_type>::value)
  {
    if (this != &o) {
      if (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
        this->destroy_resources();
        this->free_resources();

        this->m_a     = std::move(o.m_a);
        this->m_m     = std::exchange(o.m_m, nullptr);
        this->m_size  = std::exchange(o.m_size, size_type());
        this->m_msize = std::exchange(o.m_msize, size_type());
      } else {
        if (this->m_a == o.m_a) {
          this->destroy_resources();
          this->free_resources();

          this->m_m     = std::exchange(o.m_m, nullptr);
          this->m_size  = std::exchange(o.m_size, size_type());
          this->m_msize = std::exchange(o.m_msize, size_type());
        } else {
          if (this->m_msize != o.m_msize) {
            this->destroy_resources();
            this->free_resources();
            this->allocate_resources();
          }
          this->move_assign_resources_n(o.m_m, o.m_msize);
        };
      };
    };

    return *this;
  };
};

} // namespace matrices
} // namespace utilz

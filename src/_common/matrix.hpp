#pragma once

#include <map>
#include <ranges>
#include <vector>

#include <memory>
#include <utility>

namespace utilz {
namespace matrices {

// ---
// Forward declarations
//

enum clusters_indeces_flags
{
  clusters_indeces_flags_none = 0,
  clusters_indeces_flags_in_vertex   = 1,
  clusters_indeces_flags_out_vertex  = 2
};

template<typename T>
class clusters;

template<typename T, typename A>
class rect_matrix;

template<typename T, typename A>
class square_matrix;

//
// Forward declarations
// ---

template<typename T>
class clusters
{
public:
  using size_type = size_t;

private:
  std::map<T, std::vector<T>>                                m_clusters;
  std::unordered_map<T, std::vector<T>>                      m_bridges;
  std::unordered_map<T, std::vector<T>>                      m_indeces;
  std::unordered_map<T, std::vector<T>>                      m_bridges_in;
  std::unordered_map<T, std::vector<T>>                      m_indeces_in;
  std::unordered_map<T, std::vector<T>>                      m_bridges_out;
  std::unordered_map<T, std::vector<T>>                      m_indeces_out;
  std::unordered_map<T, std::vector<clusters_indeces_flags>> m_indeces_flags;
  std::unordered_map<T, T>                                   m_reverse;

public:
  // Inserts a mapping between cluster and vertex
  //
  void
  insert_map(const T& cindex, const T& vindex)
  {
    auto it = this->m_clusters.find(cindex);
    if (it == this->m_clusters.end()) {
      this->m_clusters.emplace(cindex, std::vector<T>({ vindex }));
      this->m_bridges.emplace(cindex, std::vector<T>());
      this->m_bridges_in.emplace(cindex, std::vector<T>());
      this->m_bridges_out.emplace(cindex, std::vector<T>());
      this->m_indeces.emplace(cindex, std::vector<T>());
      this->m_indeces_in.emplace(cindex, std::vector<T>());
      this->m_indeces_out.emplace(cindex, std::vector<T>());
      this->m_indeces_flags.emplace(cindex, std::vector<clusters_indeces_flags>());
    } else {
      it->second.push_back(vindex);
    }
    // Here we insert a default '0' flag for a newly detected
    // cluster vertex
    //
    this->m_indeces_flags.at(cindex).push_back(clusters_indeces_flags::clusters_indeces_flags_none);

    this->m_reverse.emplace(vindex, cindex);
  }

  // Inserts bridge information about an `edge` (requires the clusters to
  // be filled with the mappings between clusters and vertecies)
  //
  void
  insert_edge(const T& from, const T& to)
  {
    auto x = this->m_reverse.at(from);
    auto z = this->m_reverse.at(to);

    if (x != z) {
      auto& vx     = this->m_bridges.at(x);
      auto& vz     = this->m_bridges.at(z);
      auto& vz_in  = this->m_bridges_in.at(z);
      auto& vx_out = this->m_bridges_out.at(x);

      bool vx_found     = std::find(vx.begin(), vx.end(), from) == vx.end();
      bool vz_found     = std::find(vz.begin(), vz.end(), to) == vz.end();
      bool vz_in_found  = std::find(vz_in.begin(), vz_in.end(), to) == vz_in.end();
      bool vx_out_found = std::find(vx_out.begin(), vx_out.end(), from) == vx_out.end();

      if (vx_found)
        this->m_bridges.at(x).push_back(from);

      if (vx_out_found)
        this->m_bridges_out.at(x).push_back(from);

      if (vx_found || vx_out_found) {
        auto c = this->m_clusters.at(x);
        auto v = std::distance(c.begin(), std::find(c.begin(), c.end(), from));

        if (vx_found)
          this->m_indeces.at(x).push_back(v);

        if (vx_out_found) {
          this->m_indeces_out.at(x).push_back(v);
          this->m_indeces_flags.at(x)[v] = clusters_indeces_flags::clusters_indeces_flags_out_vertex;
        }
      }

      if (vz_found)
        this->m_bridges.at(z).push_back(to);

      if (vz_in_found)
        this->m_bridges_in.at(z).push_back(to);

      if (vz_found || vz_in_found) {
        auto c = this->m_clusters.at(z);
        auto v = std::distance(c.begin(), std::find(c.begin(), c.end(), to));

        if (vz_found)
          this->m_indeces.at(z).push_back(v);

        if (vz_in_found) {
          this->m_indeces_in.at(z).push_back(v);
          this->m_indeces_flags.at(z)[v] = clusters_indeces_flags::clusters_indeces_flags_in_vertex;
        }
      }
    }
  }

  size_type
  size() const noexcept
  {
    return this->m_clusters.size();
  }

  auto
  list() const noexcept
  {
    return std::views::keys(this->m_clusters);
  }

  auto
  get_bridges(const T& cindex) const
  {
    return std::views::all(this->m_bridges.at(cindex));
  }

  auto
  get_bridges_in(const T& cindex) const
  {
    return std::views::all(this->m_bridges_in.at(cindex));
  }

  auto
  get_bridges_out(const T& cindex) const
  {
    return std::views::all(this->m_bridges_out.at(cindex));
  }

  auto
  get_indeces(const T& cindex) const
  {
    return std::views::all(this->m_indeces.at(cindex));
  }

  auto
  get_indeces_in(const T& cindex) const
  {
    return std::views::all(this->m_indeces_in.at(cindex));
  }

  auto
  get_indeces_out(const T& cindex) const
  {
    return std::views::all(this->m_indeces_out.at(cindex));
  }

  auto
  get_vertices(const T& cindex) const
  {
    return std::views::all(this->m_clusters.at(cindex));
  }

  size_type
  count_vertices(const T& cindex) const
  {
    return this->m_clusters.at(cindex).size();
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

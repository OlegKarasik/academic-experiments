#pragma once

#include <memory>
#include <utility>

namespace utilz {

// ---
// Forward declarations
//

template<typename T, typename A>
class square_matrix;

//
// Forward declarations
// ---

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

    for (auto i = 0; i < n; ++i)
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

    for (auto i = 0; i < n; ++i)
      *(t++) = std::move(*(f++));
  }
  void
  destroy_resources()
  {
    pointer   p = this->m_m;
    size_type n = this->m_msize;

    for (auto i = 0; i < n; ++i)
      std::allocator_traits<A>::destroy(this->m_a, p++);
  }
  void
  free_resources()
  {
    if (this->m_msize > 0)
      std::allocator_traits<allocator_type>::deallocate(this->m_a, this->m_m, this->m_msize);
  }

public:
  square_matrix()
    : square_matrix(allocator_type())
  {
  }
  square_matrix(const allocator_type& a)
    : m_m(nullptr)
    , m_msize(0)
    , m_size(0)
    , m_a(a)
  {
  }

  square_matrix(size_type s)
    : square_matrix(s, allocator_type())
  {
  }
  square_matrix(size_type s, const allocator_type& a)
    : m_m(nullptr)
    , m_msize(s * s)
    , m_size(s)
    , m_a(a)
  {
    if (s > 0) {
      this->allocate_resources();
      this->construct_default();
    }
  }

  square_matrix(const square_matrix& o)
    : m_m(nullptr)
    , m_msize(o.m_msize)
    , m_size(o.m_size)
    , m_a(std::allocator_traits<allocator_type>::select_on_container_copy_construction(o.m_a))
  {
    this->allocate_resources();
    this->copy_insert_resources_n(o.m_m, o.m_msize);
  }
  square_matrix(square_matrix&& o) noexcept
    : m_m(std::move(o.m_m))
    , m_msize(std::exchange(o.m_msize, 0))
    , m_size(std::exchange(o.m_size, 0))
    , m_a(std::move(o.m_a))
  {
  }
  square_matrix(square_matrix&& o, const allocator_type& a)
    : m_m(nullptr)
    , m_msize(o.m_msize)
    , m_size(o.m_size)
    , m_a(a)
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

} // namespace utilz

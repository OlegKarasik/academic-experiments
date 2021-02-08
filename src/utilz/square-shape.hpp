#pragma once

#include <exception>
#include <memory>

namespace utilz {

template<typename T, typename A = std::allocator<T>>
class square_shape
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
  size_type m_side;
  pointer   m_m;

  void allocate_resources()
  {
    this->m_m = std::allocator_traits<allocator_type>::allocate(this->m_a, this->m_size);
  }
  void copy_insert_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;
    while (n >= 0) {
      std::allocator_traits<allocator_type>::construct(this->m_a, t++, *(f++));

      --n;
    }
  }
  void copy_assign_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;
    while (n >= 0) {
      *(t++) = *(f++);

      --n;
    }
  }
  void move_insert_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;
    while (n >= 0) {
      std::allocator_traits<allocator_type>::construct(this->m_a, t++, std::move(*(f++)));

      --n;
    }
  }
  void move_assign_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;
    while (n >= 0) {
      *(t++) = std::move(*(f++));

      --n;
    }
  }
  void destroy_resources()
  {
    pointer   p = this->m_m;
    size_type n = this->m_size;
    while (n >= 0) {
      std::allocator_traits<A>::destroy(this->m_a, p++);

      --n;
    }
  }
  void free_resources()
  {
    std::allocator_traits<allocator_type>::deallocate(this->m_a, this->m_m, this->m_size);
  }

public:
  square_shape(const allocator_type& a = std::allocator<T>())
    : m_m(nullptr)
    , m_a(a)
    , m_side(size_type())
    , m_size(size_type())
  {}
  square_shape(size_t side, const allocator_type& a = std::allocator<T>())
    : m_m(nullptr)
    , m_a(a)
    , m_side(side)
    , m_size(side * side)
  {
    if (side > 0)
      this->allocate_resources();
  }

  square_shape(const square_shape& o)
    : m_m(nullptr)
    , m_a(std::allocator_traits<allocator_type>::select_on_container_copy_construction(o.m_a))
    , m_side(o.m_side)
    , m_size(o.m_size)
  {
    this->allocate_resources();
    this->copy_insert_resources_n(o.m_m, o.m_size);
  }
  square_shape(square_shape&& o) noexcept
    : m_m(std::move(o.m_m))
    , m_a(std::move(o.m_a))
    , m_side(std::exchange(o.m_side, size_type()))
    , m_size(std::exchange(o.m_size, size_type()))
  {}
  square_shape(square_shape&& o, const allocator_type& a = std::allocator<T>())
    : m_m(nullptr)
    , m_a(a)
    , m_side(o.m_side)
    , m_size(o.m_size)
  {
    if (this->m_size > 0) {
      if (this->m_a == o.m_a) {
        this->m_m = std::move(o.m_m);
      } else {
        this->allocate_resources();
        this->move_insert_resources_n(o.m_m, o.m_size);

        // Release resources and reset members to ensure
        // no object will be 'destroyed' in 'destructor'
        //
        o.free_resources();
      }

      o.m_size = size_type();
      o.m_side = size_type();
      o.m_m    = nullptr;
    }
  }

  bool empty() const
  {
    return this->m_size == 0;
  }

  size_type side() const
  {
    return this->m_side;
  }
  size_type size() const
  {
    return this->m_size;
  }

  pointer at(size_type i) noexcept
  {
    return &this->m_m[i * this->m_side];
  }
  const_pointer at(size_type i) const noexcept
  {
    return &this->m_m[i * this->m_side];
  }

  reference at(size_type i, size_type j) noexcept
  {
    return this->m_m[i * this->m_side + j];
  }
  const_reference at(size_type i, size_type j) const noexcept
  {
    return this->m_m[i * this->m_side + j];
  }

  bool operator==(const square_shape& o) const noexcept
  {
    return this == &o || (this->m_size == o.m_size && std::equal(this->m_m, this->m_m + this->m_size, o.m_m));
  };
  bool operator!=(const square_shape& o) const noexcept
  {
    return !(*this == o);
  };

  square_shape& operator=(const square_shape& o)
  {
    if (this != &o) {
      this->destroy_resources();
      this->free_resources();

      if (std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value)
        this->m_a = o.m_a;

      this->m_size = o.m_size;
      this->m_side = o.m_side;

      this->allocate_resources();
      this->copy_assign_resources_n(o.m_m, o.m_size);
    }
    return *this;
  }
  square_shape& operator=(square_shape&& o) noexcept(
    std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value&& std::is_nothrow_move_assignable<allocator_type>::value)
  {
    if (this != &o) {
      if (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value) {
        this->destroy_resources();
        this->free_resources();

        this->m_a    = std::move(o.m_a);
        this->m_m    = std::exchange(o.m_m, nullptr);
        this->m_side = std::exchange(o.m_side, size_type());
        this->m_size = std::exchange(o.m_size, size_type());
      } else {
        if (this->m_a == o.m_a) {
          this->destroy_resources();
          this->free_resources();

          this->m_m    = std::exchange(o.m_m, nullptr);
          this->m_side = std::exchange(o.m_side, size_type());
          this->m_size = std::exchange(o.m_size, size_type());
        } else {
          if (this->m_size != o.m_size) {
            this->destroy_resources();
            this->free_resources();
            this->allocate_resources();
          }
          this->move_assign_resources_n(o.m_m, o.m_size);
        };
      };
    };

    return *this;
  };
};

} // namespace utilz

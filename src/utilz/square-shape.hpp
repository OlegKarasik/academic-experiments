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
  size_type m_width;
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
    , m_width(size_type())
    , m_size(size_type())
  {}
  square_shape(size_t side, const allocator_type& a = std::allocator<T>())
    : m_m(nullptr)
    , m_a(a)
    , m_width(side)
    , m_size(side * side)
  {
    if (side > 0)
      this->allocate_resources();
  }

  square_shape(const square_shape& o)
    : m_m(nullptr)
    , m_a(std::allocator_traits<allocator_type>::select_on_container_copy_construction(o.m_a))
    , m_width(o.m_width)
    , m_size(o.m_size)
  {
    this->allocate_resources();
    this->copy_insert_resources_n(o.m_m, o.m_size);
  }
  square_shape(square_shape&& o) noexcept
    : m_m(std::move(o.m_m))
    , m_a(std::move(o.m_a))
    , m_width(std::exchange(o.m_width, size_type()))
    , m_size(std::exchange(o.m_size, size_type()))
  {}
  square_shape(square_shape&& o, const allocator_type& a = std::allocator<T>())
    : m_m(nullptr)
    , m_a(a)
    , m_width(o.m_width)
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

      o.m_size  = size_type();
      o.m_width = size_type();
      o.m_m     = nullptr;
    }
  }

  bool empty() const
  {
    return this->m_size == 0;
  }

  size_type width() const
  {
    return this->m_width;
  }

  pointer at(size_type i) noexcept
  {
    return &this->m_m[i * this->m_width];
  }
  const_pointer at(size_type i) const noexcept
  {
    return &this->m_m[i * this->m_width];
  }

  reference at(size_type i, size_type j) noexcept
  {
    return this->m_m[i * this->m_width + j];
  }
  const_reference at(size_type i, size_type j) const noexcept
  {
    return this->m_m[i * this->m_width + j];
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

      this->m_size  = o.m_size;
      this->m_width = o.m_width;

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

        this->m_a     = std::move(o.m_a);
        this->m_m     = std::exchange(o.m_m, nullptr);
        this->m_width = std::exchange(o.m_width, size_type());
        this->m_size  = std::exchange(o.m_size, size_type());
      } else {
        if (this->m_a == o.m_a) {
          this->destroy_resources();
          this->free_resources();

          this->m_m     = std::exchange(o.m_m, nullptr);
          this->m_width = std::exchange(o.m_width, size_type());
          this->m_size  = std::exchange(o.m_size, size_type());
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

namespace transforms {

template<typename Operation, typename Shape>
class square_shape_at
{
public:
  using shape_type = Shape;

private:
  Operation& m_t;

public:
  square_shape_at(Operation& t)
    : m_t(t)
  {}

  void operator()(shape_type& s, size_t i, size_t j)
  {
    return this->m_t(s.at(i, j));
  }
};

template<typename Operation>
square_shape_at(Operation& t)
  -> square_shape_at<Operation, square_shape<typename Operation::result_type>>;

// This transform is intended to be used with
// nested square_shapes i.e. when T is not a type but
// a square_shape<U>
//
template<typename Transform, typename Shape>
class square_shape_drill_at
{
public:
  using shape_type = Shape;

private:
  Transform& m_t;

  size_t m_w;

public:
  square_shape_drill_at(Transform& t, size_t w)
    : m_t(t)
    , m_w(w)
  {
  }

  void operator()(Shape& s, size_t i, size_t j)
  {
    this->m_t(s.at(i / this->m_w, j / this->m_w), i % this->m_w, j % this->m_w);
  }
};

template<typename Transform>
square_shape_drill_at(Transform& t, size_t w)
  -> square_shape_drill_at<Transform, square_shape<typename Transform::shape_type>>;

} // namespace transforms

} // namespace utilz

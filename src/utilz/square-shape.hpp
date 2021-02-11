#pragma once

#include <exception>
#include <memory>
#include <type_traits>

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

  size_type m_msize;
  pointer   m_m;

  void allocate_resources()
  {
    this->m_m = std::allocator_traits<allocator_type>::allocate(this->m_a, this->m_msize);
  }
  void construct_default()
  {
    pointer   t = this->m_m;
    size_type n = this->m_msize;

    for (auto i = 0; i < n; ++i)
      std::allocator_traits<allocator_type>::construct(this->m_a, t++);
  }
  void copy_insert_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = 0; i < n; ++i)
      std::allocator_traits<allocator_type>::construct(this->m_a, t++, *(f++));
  }
  void copy_assign_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = 0; i < n; ++i)
      *(t++) = *(f++);
  }
  void move_insert_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = 0; i < n; ++i)
      std::allocator_traits<allocator_type>::construct(this->m_a, t++, std::move(*(f++)));
  }
  void move_assign_resources_n(pointer f, size_type n)
  {
    pointer t = this->m_m;

    for (auto i = 0; i < n; ++i)
      *(t++) = std::move(*(f++));
  }
  void destroy_resources()
  {
    pointer   p = this->m_m;
    size_type n = this->m_msize;

    for (auto i = 0; i < n; ++i)
      std::allocator_traits<A>::destroy(this->m_a, p++);
  }
  void free_resources()
  {
    std::allocator_traits<allocator_type>::deallocate(this->m_a, this->m_m, this->m_msize);
  }

public:
  square_shape()
    : m_m(nullptr)
    , m_a(allocator_type())
    , m_size(size_type())
    , m_msize(size_type())
  {}
  square_shape(const allocator_type& a)
    : m_m(nullptr)
    , m_a(a)
    , m_size(size_type())
    , m_msize(size_type())
  {}
  square_shape(size_t s, const allocator_type& a = std::allocator<T>())
    : m_m(nullptr)
    , m_a(a)
    , m_size(s)
    , m_msize(s * s)
  {
    if (s > 0) {
      this->allocate_resources();
      this->construct_default();
    }
  }

  square_shape(const square_shape& o)
    : m_m(nullptr)
    , m_a(std::allocator_traits<allocator_type>::select_on_container_copy_construction(o.m_a))
    , m_size(o.m_size)
    , m_msize(o.m_msize)
  {
    this->allocate_resources();
    this->copy_insert_resources_n(o.m_m, o.m_msize);
  }
  square_shape(square_shape&& o) noexcept
    : m_m(std::move(o.m_m))
    , m_a(std::move(o.m_a))
    , m_size(std::exchange(o.m_size, size_type()))
    , m_msize(std::exchange(o.m_msize, size_type()))
  {}
  square_shape(square_shape&& o, const allocator_type& a = std::allocator<T>())
    : m_m(nullptr)
    , m_a(a)
    , m_size(o.m_size)
    , m_msize(o.m_msize)
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
  }

  bool empty() const
  {
    return this->m_msize == 0;
  }

  size_type size() const
  {
    return this->m_size;
  }

  pointer at(size_type i) noexcept
  {
    return &this->m_m[i * this->m_size];
  }
  const_pointer at(size_type i) const noexcept
  {
    return &this->m_m[i * this->m_size];
  }

  reference at(size_type i, size_type j) noexcept
  {
    return this->m_m[i * this->m_size + j];
  }
  const_reference at(size_type i, size_type j) const noexcept
  {
    return this->m_m[i * this->m_size + j];
  }

  bool operator==(const square_shape& o) const noexcept
  {
    return this == &o || (this->m_msize == o.m_msize && std::equal(this->m_m, this->m_m + this->m_msize, o.m_m));
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

      this->m_msize = o.m_msize;
      this->m_size  = o.m_size;

      this->allocate_resources();
      this->copy_assign_resources_n(o.m_m, o.m_msize);
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

namespace traits {

template<typename>
struct is_square_shape : public std::false_type
{};

template<typename T, typename A>
struct is_square_shape<square_shape<T, A>> : public std::true_type
{};

} // namespace traits

namespace procedures {

template<typename Shape>
class square_shape_size
{
  static_assert(traits::is_square_shape<Shape>::value, "Operation requires square_shape");

public:
  using result_type = typename Shape::size_type;

private:
  template<typename Q = Shape>
  result_type proc(const std::enable_if_t<traits::is_square_shape<typename Q::value_type>::value, Q>& s)
  {
    square_shape_size<typename Q::value_type> in_proc;
    return s.size() * in_proc(s.at(0,0));
  };
  template<typename Q = Shape>
  result_type proc(const std::enable_if_t<!traits::is_square_shape<typename Q::value_type>::value, Q>& s)
  {
    return s.size();
  }

public:
  result_type operator()(const Shape& s)
  {
    return proc(s);
  }
};

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

  void operator()(shape_type& s, typename Shape::size_type i, typename Shape::size_type j)
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

  typename Shape::size_type m_s;

public:
  square_shape_drill_at(Transform& t, typename Shape::size_type s)
    : m_t(t)
    , m_s(s)
  {
  }

  void operator()(Shape& s, typename Shape::size_type i, typename Shape::size_type j)
  {
    this->m_t(s.at(i / this->m_s, j / this->m_s), i % this->m_s, j % this->m_s);
  }
};

template<typename Transform>
square_shape_drill_at(Transform& t, typename Transform::shape_type::size_type s)
  -> square_shape_drill_at<Transform, square_shape<typename Transform::shape_type>>;

} // namespace procedures

} // namespace utilz

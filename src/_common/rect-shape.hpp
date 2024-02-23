#pragma once

#include <memory>
#include <utility>

namespace utilz {

// ---
// Forward declarations
//

template<typename T, typename A>
class rect_shape;

namespace traits {

template<typename S>
struct rect_shape_traits;

template<typename T, typename A>
struct rect_shape_traits<rect_shape<T, A>>;

} // namespace traits

namespace procedures {

namespace get_size {

template<typename S>
struct impl;

} // namespace get_size

namespace set_size {

template<std::size_t I, typename S>
struct impl;

} // namespace set_size

template<typename S>
using rect_shape_get_size = get_size::impl<S>;

template<typename S>
using rect_shape_set_size = set_size::impl<std::size_t(0), S>;

template<typename S>
struct rect_shape_at;

template<typename S>
struct rect_shape_get;

template<typename S>
struct rect_shape_set;

template<typename S>
struct rect_shape_replace;

} // namespace procedures

//
// Forward declarations
// ---

template<typename T, typename A = std::allocator<T>>
class rect_shape
{
#ifdef _INTEL_COMPILER
  template<std::size_t I, typename S>
#else
  template<std::size_t I, T>
#endif
  friend struct ::utilz::procedures::set_size::impl;

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
  rect_shape()
    : m_m(nullptr)
    , m_msize(0)
    , m_width(0)
    , m_height(0)
    , m_a(allocator_type())
  {
  }
  rect_shape(const allocator_type& a)
    : m_m(nullptr)
    , m_msize(0)
    , m_width(0)
    , m_height(0)
    , m_a(a)
  {
  }

  rect_shape(size_type w, size_type h, const allocator_type& a)
    : m_m(nullptr)
    , m_msize(w * h)
    , m_width(w)
    , m_height(h)
    , m_a(a)
  {
    if ((w * h) > 0) {
      this->allocate_resources();
      this->construct_default();
    }
  }

  rect_shape(const rect_shape& o)
    : m_m(nullptr)
    , m_msize(o.m_msize)
    , m_width(o.m_width)
    , m_height(o.m_height)
    , m_a(std::allocator_traits<allocator_type>::select_on_container_copy_construction(o.m_a))
  {
    this->allocate_resources();
    this->copy_insert_resources_n(o.m_m, o.m_msize);
  }
  rect_shape(rect_shape&& o) noexcept
    : m_m(std::move(o.m_m))
    , m_msize(std::exchange(o.m_msize, 0))
    , m_width(std::exchange(o.m_width, 0))
    , m_height(std::exchange(o.m_height, 0))
    , m_a(std::move(o.m_a))
  {
  }
  rect_shape(rect_shape&& o, const allocator_type& a)
    : m_m(nullptr)
    , m_msize(o.m_msize)
    , m_width(o.m_width)
    , m_height(o.m_height)
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

      o.m_msize  = size_type();
      o.m_width  = size_type();
      o.m_height = size_type();
      o.m_m      = nullptr;
    }
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
  operator==(const rect_shape& o) const noexcept
  {
    return this == &o || (this->m_msize == o.m_msize && std::equal(this->m_m, this->m_m + this->m_msize, o.m_m));
  };
  bool
  operator!=(const rect_shape& o) const noexcept
  {
    return !(*this == o);
  };

  rect_shape&
  operator=(const rect_shape& o)
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
  rect_shape&
  operator=(rect_shape&& o) noexcept(
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

namespace traits {

template<typename T>
struct rect_shape_traits
{
public:
  using is = std::bool_constant<false>;
};

template<typename T, typename A>
struct rect_shape_traits<rect_shape<T, A>>
{
public:
  using is         = std::bool_constant<true>;
  using item_type  = typename rect_shape<T, A>::value_type;
  using size_type  = typename rect_shape<T, A>::size_type;
  using value_type = typename rect_shape<T, A>::value_type;
  using pointer    = typename rect_shape<T, A>::pointer;
};

} // namespace traits

namespace procedures {

namespace get_size {

template<typename S>
struct impl
{
  static_assert(traits::rect_shape_traits<S>::is::value, "erro: input type has to be a rect_shape of T");
};

template<typename T, typename A>
struct impl<rect_shape<T, A>>
{
public:
  using result_type = typename traits::rect_shape_traits<rect_shape<T, A>>::size_type;

public:
  result_type
  operator()(const rect_shape<T, A>& s)
  {
    return s.size();
  }
};

} // namespace get_size

namespace set_size {

template<std::size_t I, typename T>
struct __value
{
public:
  T value;
};

template<std::size_t I, typename S>
struct impl
{
  static_assert(traits::rect_shape_traits<S>::is::value, "erro: input type has to be a rect_shape of T");
};

template<std::size_t I, typename T, typename A>
struct impl<I, rect_shape<T, A>>
{
public:
  using result_type = typename traits::rect_shape_traits<rect_shape<T, A>>::size_type;

public:
  void
  operator()(rect_shape<T, A>& s, result_type sz)
  {
    s = square_shape<T, A>(sz, s.m_a);
  }
};

} // namespace set_size

template<typename S>
struct rect_shape_at
{
  static_assert(traits::rect_shape_traits<S>::is::value, "erro: input type has to be a rect_shape of T");
};

template<typename T, typename A>
struct rect_shape_at<rect_shape<T, A>>
{
private:
  using size_type = typename traits::rect_shape_traits<rect_shape<T, A>>::size_type;

public:
  using result_type = typename traits::rect_shape_traits<rect_shape<T, A>>::value_type;

public:
  result_type&
  operator()(rect_shape<T, A>& s, size_type i, size_type j)
  {
    return s.at(i, j);
  }
  const result_type&
  operator()(const rect_shape<T, A>& s, size_type i, size_type j)
  {
    return s.at(i, j);
  }
};

template<typename S>
struct rect_shape_get
{
private:
  static_assert(traits::square_shape_traits<S>::is::value, "erro: input type has to be a rect_shape_get of T");

private:
  using size_type = typename traits::square_shape_traits<S>::size_type;

public:
  using result_type = typename traits::square_shape_traits<S>::value_type;

private:
  square_shape_at<S> m_at;

public:
  result_type
  operator()(const S& s, size_type i, size_type j)
  {
    return this->m_at(s, i, j);
  }
};

template<typename S>
struct rect_shape_set
{
private:
  static_assert(traits::rect_shape_traits<S>::is::value, "erro: input type has to be a rect_shape of T");

private:
  using size_type = typename traits::rect_shape_traits<S>::size_type;

public:
  using result_type = typename traits::rect_shape_traits<S>::value_type;

private:
  rect_shape_at<S> m_at;

public:
  void
  operator()(S& s, size_type i, size_type j, result_type v)
  {
    this->m_at(s, i, j) = v;
  }
};

template<typename S>
struct rect_shape_replace
{
private:
  static_assert(traits::rect_shape_traits<S>::is::value, "erro: input type has to be a rect_shape of T");

private:
  using size_type  = typename traits::rect_shape_traits<S>::size_type;
  using value_type = typename traits::rect_shape_traits<S>::value_type;

public:
  using result_type = typename traits::rect_shape_traits<S>::value_type;

private:
  rect_shape_get_size<S> m_size;
  rect_shape_at<S>       m_at;

public:
  void
  operator()(S& s, value_type f, value_type t)
  {
    size_type size = this->m_size(s);

    for (size_type i = size_type(0); i < size; ++i)
      for (size_type j = size_type(0); j < size; ++j)
        if (this->m_at(s, i, j) == f)
          this->m_at(s, i, j) = t;
  }
};

} // namespace procedures

} // namespace utilz

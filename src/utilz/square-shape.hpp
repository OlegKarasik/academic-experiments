#pragma once

#include <exception>
#include <memory>
#include <type_traits>

namespace utilz {

// ---
// Forward declarations
//

template<typename T, typename A>
class square_shape;

namespace traits {

template<typename S>
struct square_shape_traits;

template<typename T, typename A>
struct square_shape_traits<square_shape<T, A>>;

} // namespace traits

namespace procedures {

template<typename S>
struct square_shape_size;

template<typename S>
struct square_shape_at;

} // namespace procedures

//
// Forward declarations
// ---

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
    std::allocator_traits<allocator_type>::deallocate(this->m_a, this->m_m, this->m_msize);
  }

public:
  square_shape()
    : m_m(nullptr)
    , m_msize(0)
    , m_size(0)
    , m_a(allocator_type())
  {}
  square_shape(const allocator_type& a)
    : m_m(nullptr)
    , m_msize(0)
    , m_size(0)
    , m_a(a)
  {}

  square_shape(size_type s, const allocator_type& a = std::allocator<T>())
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

  square_shape(const square_shape& o)
    : m_m(nullptr)
    , m_msize(o.m_msize)
    , m_size(o.m_size)
    , m_a(std::allocator_traits<allocator_type>::select_on_container_copy_construction(o.m_a))
  {
    this->allocate_resources();
    this->copy_insert_resources_n(o.m_m, o.m_msize);
  }
  square_shape(square_shape&& o) noexcept
    : m_m(std::move(o.m_m))
    , m_msize(std::exchange(o.m_msize, 0))
    , m_size(std::exchange(o.m_size, 0))
    , m_a(std::move(o.m_a))
  {}
  square_shape(square_shape&& o, const allocator_type& a = std::allocator<T>())
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
  }

  bool
  empty() const
  {
    return this->m_msize == 0;
  }

  size_type
  size() const
  {
    return this->m_size;
  }

  pointer
  at(size_type i) noexcept
  {
    return &this->m_m[i * this->m_size];
  }
  const_pointer
  at(size_type i) const noexcept
  {
    return &this->m_m[i * this->m_size];
  }

  reference
  at(size_type i, size_type j) noexcept
  {
    return this->m_m[i * this->m_size + j];
  }
  const_reference
  at(size_type i, size_type j) const noexcept
  {
    return this->m_m[i * this->m_size + j];
  }

  bool
  operator==(const square_shape& o) const noexcept
  {
    return this == &o || (this->m_msize == o.m_msize && std::equal(this->m_m, this->m_m + this->m_msize, o.m_m));
  };
  bool
  operator!=(const square_shape& o) const noexcept
  {
    return !(*this == o);
  };

  square_shape&
  operator=(const square_shape& o)
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
  square_shape&
  operator=(square_shape&& o) noexcept(
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

template<typename T>
struct square_shape_traits
{
public:
  using is         = std::bool_constant<false>;
  using value_type = T;
};

template<typename T, typename A>
struct square_shape_traits<square_shape<T, A>>
{
public:
  using is         = std::bool_constant<true>;
  using item_type  = typename square_shape<T, A>::value_type;
  using value_type = typename square_shape_traits<typename square_shape<T, A>::value_type>::value_type;
  using size_type  = typename square_shape<T, A>::size_type;
};

} // namespace traits

namespace procedures {

template<typename S>
struct square_shape_size
{
private:
  template<typename T>
  using _traits = traits::square_shape_traits<T>;

  static_assert(_traits<S>::is::value, "erro: input type has to be a `square_shape<T>`");

private:
  template<typename T>
  using _item_type = typename _traits<T>::item_type;

public:
  using result_type = typename _traits<S>::size_type;

private:
  template<typename Q = S, std::enable_if_t<!_traits<_item_type<Q>>::is::value, bool> = true>
  result_type
  invoke(const Q& s)
  {
    return s.size();
  }

  template<typename Q = S, std::enable_if_t<_traits<_item_type<Q>>::is::value, bool> = true>
  result_type
  invoke(const Q& s)
  {
    square_shape_size<_item_type<Q>> procedure;

    return s.size() * procedure(s.at(0, 0));
  }

public:
  result_type
  operator()(const S& s)
  {
    if (s.size() == 0)
      return 0;

    return invoke(s);
  }
};

template<typename S>
struct square_shape_at
{
private:
  template<typename __S>
  using __traits = traits::square_shape_traits<__S>;

  static_assert(__traits<S>::is::value, "erro: input type has to be a `square_shape<T>`");

private:
  template<typename T>
  using _item_type = typename __traits<T>::item_type;

  template<typename T>
  using _size_type = typename __traits<T>::size_type;

public:
  using result_type = typename __traits<S>::value_type;

private:
  template<typename Q = S, std::enable_if_t<!__traits<_item_type<Q>>::is::value, bool> = true>
  result_type&
  invoke(Q& s, _size_type<Q> i, _size_type<Q> j)
  {
    return s.at(i, j);
  }

  template<typename Q = S, std::enable_if_t<__traits<_item_type<Q>>::is::value, bool> = true>
  result_type&
  invoke(Q& s, _size_type<Q> i, _size_type<Q> j)
  {
    square_shape_at<_item_type<Q>> procedure;

    return procedure(s.at(i / s.size(), j / s.size()), i % s.size(), j % s.size());
  }

  template<typename Q = S, std::enable_if_t<!__traits<_item_type<Q>>::is::value, bool> = true>
  const result_type&
  invoke(const Q& s, _size_type<Q> i, _size_type<Q> j)
  {
    return s.at(i, j);
  }

  template<typename Q = S, std::enable_if_t<__traits<_item_type<Q>>::is::value, bool> = true>
  const result_type&
  invoke(const Q& s, _size_type<Q> i, _size_type<Q> j)
  {
    square_shape_at<_item_type<Q>> procedure;

    return procedure(s.at(i / s.size(), j / s.size()), i % s.size(), j % s.size());
  }

public:
  result_type&
  operator()(S& s, _size_type<S> i, _size_type<S> j)
  {
    return invoke(s, i, j);
  }
  const result_type&
  operator()(const S& s, _size_type<S> i, _size_type<S> j)
  {
    return invoke(s, i, j);
  }
};

} // namespace procedures

} // namespace utilz

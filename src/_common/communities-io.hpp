#pragma once

#include <istream>
#include <memory>
#include <ostream>
#include <stdexcept>

namespace utilz {
namespace communities {
namespace io {

// ---
// Forward declarations
//

enum communities_format
{
  communities_fmt_none  = 0,
  communities_fmt_rlang = 1
};

enum communities_preamble_format
{
  communities_preamble_fmt_none = 0,
  communities_preamble_fmt_index = 1
};

namespace impl {

template<typename TIndex>
class communities_preamble;

template<communities_format F, typename C, typename V>
void
scan_communities(
  std::istream& is,
  C&            communities,
  V&            set_v,
  std::integral_constant<communities_preamble_format, communities_preamble_format::communities_preamble_fmt_index>);

template<communities_format F, typename C, typename V>
void
scan_communities(
  std::istream& is,
  C&            communities,
  V&            set_w);

} // namespace impl

template<communities_format TFormat>
class communities_traits
{
  static_assert(false, "The format is not supported");
};

template<communities_format TFormat, typename TIndex>
class communities_preamble
{
  static_assert(false, "The format is not supported");
};

template<>
class communities_traits<communities_format::communities_fmt_rlang>
{
public:
  using preamble_format = std::integral_constant<communities_preamble_format, communities_preamble_format::communities_preamble_fmt_index>;
};

template<typename TIndex>
std::istream&
operator>>(std::istream& is, communities_preamble<communities_format::communities_fmt_rlang, TIndex>& preamble);

template<typename TIndex>
std::ostream&
operator<<(std::ostream& os, const communities_preamble<communities_format::communities_fmt_rlang, TIndex>& preamble);

template<typename TIndex>
class communities_preamble<communities_format::communities_fmt_rlang, TIndex> : public impl::communities_preamble<TIndex>
{
public:
  communities_preamble()
    : impl::communities_preamble<TIndex>()
  {
  }

  communities_preamble(TIndex index)
    : impl::communities_preamble<TIndex>(index)
  {
  }

  friend std::istream&
  operator>><TIndex>(std::istream& is, communities_preamble<communities_format::communities_fmt_rlang, TIndex>& preamble);

  friend std::ostream&
  operator<<<TIndex>(std::ostream& os, const communities_preamble<communities_format::communities_fmt_rlang, TIndex>& preamble);
};

template<typename TIndex>
std::istream&
operator>>(std::istream& is, communities_preamble<communities_format::communities_fmt_rlang, TIndex>& preamble)
{
  return is;
};

template<typename TIndex>
std::ostream&
operator<<(std::ostream& os, const communities_preamble<communities_format::communities_fmt_rlang, TIndex>& preamble)
{
  return os;
};

//
// Forward declarations
// ---

bool
parse_communities_format(
  const std::string&  format,
  communities_format& out_format)
{
  if (format == "rlang") {
    out_format = communities_format::communities_fmt_rlang;
    return true;
  }
  return false;
};

template<typename C, typename V>
void
scan_communities(
  communities_format format,
  std::istream&      is,
  C&                 communities,
  V&                 set_v)
{
  switch (format) {
    case communities_format::communities_fmt_rlang:
      impl::scan_communities<communities_format::communities_fmt_rlang>(is, communities, set_v);
      break;
    default:
      throw std::logic_error("erro: The format is not supported");
  }
};

namespace impl {

template<typename TIndex>
class communities_preamble
{
private:
  TIndex m_idx;

public:
  communities_preamble()
    : m_idx(TIndex(0))
  {
  }

  communities_preamble(TIndex index)
    : m_idx(index)
  {
  }

  TIndex
  index() const
  {
    return this->m_idx;
  }
};

template<communities_format F, typename C, typename V>
void
scan_communities(
  std::istream& is,
  C&            communities,
  V&            set_v,
  std::integral_constant<communities_preamble_format, communities_preamble_format::communities_preamble_fmt_index>)
{
  using st = typename C::size_type;

  io::communities_preamble<F, st> preamble;
  if (!(is >> preamble))
    throw std::logic_error("erro: can't scan 'communities_preamble' because of invalid format or IO problem");
};

template<communities_format F, typename C, typename V>
void
scan_communities(
  std::istream& is,
  C&            communities,
  V&            set_v)
{
  scan_communities<F, C, V>(
    is,
    communities,
    set_v,
    typename communities_traits<F>::preamble_format());
};

} // namespace impl

} // namespace io
} // namespace communities
} // namespace utilz

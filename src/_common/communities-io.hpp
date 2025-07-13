#pragma once

#include <functional>
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
  communities_preamble_fmt_none  = 0,
  communities_preamble_fmt_index = 1
};

namespace impl {

template<typename TIndex>
class communities_preamble;

template<typename TIndex>
class communities_items;

template<typename I>
std::map<I, std::vector<I>>
scan_communities(
  std::istream& is,
  std::integral_constant<communities_preamble_format, communities_preamble_format::communities_preamble_fmt_index>);

template<communities_format F, typename I>
std::map<I, std::vector<I>>
scan_communities(
  std::istream& is);

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

template<communities_format TFormat, typename TIndex>
class communities_items
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
std::istream&
operator>>(std::istream& is, communities_items<communities_format::communities_fmt_rlang, TIndex>& items);

template<typename TIndex>
std::ostream&
operator<<(std::ostream& os, const communities_items<communities_format::communities_fmt_rlang, TIndex>& items);

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
class communities_items<communities_format::communities_fmt_rlang, TIndex> : public impl::communities_items<TIndex>
{
public:
  communities_items()
    : impl::communities_items<TIndex>()
  {
  }

  communities_items(std::vector<TIndex> items)
    : impl::communities_items<TIndex>(items)
  {
  }

  friend std::istream&
  operator>><TIndex>(std::istream& is, communities_items<communities_format::communities_fmt_rlang, TIndex>& item);

  friend std::ostream&
  operator<<<TIndex>(std::ostream& os, const communities_items<communities_format::communities_fmt_rlang, TIndex>& item);
};

template<typename TIndex>
std::istream&
operator>>(std::istream& is, communities_preamble<communities_format::communities_fmt_rlang, TIndex>& preamble)
{
  std::string line;
  if (std::getline(is, line)) {
    TIndex index;
    char   sign, open, close;

    std::stringstream ss(line);
    if (ss >> sign >> open >> index >> close) {
      preamble = communities_preamble<communities_format::communities_fmt_rlang, TIndex>(--index);
      return is;
    }
  }

  is.setstate(std::ios_base::failbit);
  return is;
};

template<typename TIndex>
std::ostream&
operator<<(std::ostream& os, const communities_preamble<communities_format::communities_fmt_rlang, TIndex>& preamble)
{
  return os;
};

template<typename TIndex>
std::istream&
operator>>(std::istream& is, communities_items<communities_format::communities_fmt_rlang, TIndex>& items)
{
  std::vector<TIndex> indexes;

  std::string line;
  while (std::getline(is, line)) {
    // The format dictates that all communities are separated by an
    // empty line, so when we have one we return success
    //
    if (line.empty()) {
      items = communities_items<communities_format::communities_fmt_rlang, TIndex>(indexes);
      return is;
    }

    TIndex key;
    char   open, close;

    std::stringstream ss(line);
    if (ss >> open >> key >> close) {
      TIndex index;
      while (ss >> index)
        indexes.push_back(--index);
    }
  }

  is.setstate(std::ios_base::failbit);
  return is;
};

template<typename TIndex>
std::ostream&
operator<<(std::ostream& os, const communities_items<communities_format::communities_fmt_rlang, TIndex>& item)
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

template<typename I>
std::map<I, std::vector<I>>
scan_communities(
  communities_format format,
  std::istream&      is)
{
  switch (format) {
    case communities_format::communities_fmt_rlang:
      return impl::scan_communities<communities_format::communities_fmt_rlang, I>(is);
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

template<typename TIndex>
class communities_items
{
private:
  std::vector<TIndex> m_items;

public:
  communities_items()
  {
  }

  communities_items(std::vector<TIndex> items)
    : m_items(items)
  {
  }

  std::vector<TIndex>&
  items()
  {
    return this->m_items;
  }
};

template<communities_format F, typename I>
std::map<I, std::vector<I>>
scan_communities(
  std::istream& is,
  std::integral_constant<communities_preamble_format, communities_preamble_format::communities_preamble_fmt_index>)
{
  std::map<I, std::vector<I>> communities;
  for (;;) {
    io::communities_preamble<F, I> preamble;
    if (!(is >> preamble)) {
      // We might have reached the end of file and therefore it is not a failure to
      // read a preamble.
      //
      if (is.eof())
        break;

      // Otherwise, we throw an exception to ensure the issue is propagated
      //
      throw std::logic_error("erro: can't scan 'communities_preamble' because of invalid format or IO problem");
    }

    io::communities_items<F, I> items;
    if (!(is >> items))
      throw std::logic_error("erro: can't scan 'communities_items' because of invalid format or IO problem");

    communities.emplace(preamble.index(), items.items());
  }
  return communities;
};

template<communities_format F, typename I>
std::map<I, std::vector<I>>
scan_communities(
  std::istream& is)
{
  return scan_communities<F, I>(
    is,
    typename communities_traits<F>::preamble_format());
};

} // namespace impl

} // namespace io
} // namespace communities
} // namespace utilz

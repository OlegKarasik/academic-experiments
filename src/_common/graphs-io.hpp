#pragma once

#include <functional>
#include <istream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <algorithm>

#ifdef __clang__
#include <sstream>
#endif

namespace utilz {
namespace graphs {
namespace io {

template<typename>
struct is_tuple : std::false_type
{
};

template<typename... T>
struct is_tuple<std::tuple<T...>> : std::true_type
{
};

// ---
// Forward declarations
//

enum graph_format
{
  graph_fmt_none       = 0,
  graph_fmt_edgelist   = 1,
  graph_fmt_weightlist = 2,
  graph_fmt_dimacs     = 3,
  graph_fmt_binary     = 4
};

enum graph_preamble_format
{
  graph_preamble_fmt_none         = 0,
  graph_preamble_fmt_vertex_count = 1,
  graph_preamble_fmt_edge_count   = 2,
  graph_preamble_fmt_full         = 3
};

template<graph_format TFormat>
class graph_traits
{
  static_assert(false, "The format is not supported");
};

template<graph_format TFormat, typename TIndex>
class graph_preamble
{
  static_assert(false, "The format is not supported");
};

template<graph_format TFormat, typename TIndex, typename TWeight>
class graph_edge
{
  static_assert(false, "The format is not supported");
};

template<>
class graph_traits<graph_format::graph_fmt_edgelist>
{
public:
  using preamble_format = std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>;
};

template<>
class graph_traits<graph_format::graph_fmt_weightlist>
{
public:
  using preamble_format = std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>;
};

template<>
class graph_traits<graph_format::graph_fmt_dimacs>
{
public:
  using preamble_format = std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>;
};

template<>
class graph_traits<graph_format::graph_fmt_binary>
{
public:
  using preamble_format = std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>;
};

template<typename TIndex>
std::istream&
operator>>(std::istream& is, graph_preamble<graph_format::graph_fmt_dimacs, TIndex>& preamble);

template<typename TIndex>
std::istream&
operator>>(std::istream& is, graph_preamble<graph_format::graph_fmt_binary, TIndex>& preamble);

template<typename TIndex>
std::ostream&
operator<<(std::ostream& os, const graph_preamble<graph_format::graph_fmt_dimacs, TIndex>& preamble);

template<typename TIndex>
std::ostream&
operator<<(std::ostream& os, const graph_preamble<graph_format::graph_fmt_binary, TIndex>& preamble);

template<typename TIndex, typename TWeight>
std::istream&
operator>>(std::istream& is, graph_edge<graph_format::graph_fmt_edgelist, TIndex, TWeight>& edge);

template<typename TIndex, typename TWeight>
std::istream&
operator>>(std::istream& is, graph_edge<graph_format::graph_fmt_weightlist, TIndex, TWeight>& edge);

template<typename TIndex, typename TWeight>
std::istream&
operator>>(std::istream& is, graph_edge<graph_format::graph_fmt_dimacs, TIndex, TWeight>& edge);

template<typename TIndex, typename TWeight>
std::istream&
operator>>(std::istream& is, graph_edge<graph_format::graph_fmt_binary, TIndex, TWeight>& edge);

template<typename TIndex, typename TWeight>
std::ostream&
operator<<(std::ostream& os, const graph_edge<graph_format::graph_fmt_edgelist, TIndex, TWeight>& edge);

template<typename TIndex, typename TWeight>
std::ostream&
operator<<(std::ostream& os, const graph_edge<graph_format::graph_fmt_weightlist, TIndex, TWeight>& edge);

template<typename TIndex, typename TWeight>
std::ostream&
operator<<(std::ostream& os, const graph_edge<graph_format::graph_fmt_dimacs, TIndex, TWeight>& edge);

template<typename TIndex, typename TWeight>
std::ostream&
operator<<(std::ostream& os, const graph_edge<graph_format::graph_fmt_binary, TIndex, TWeight>& edge);

namespace impl {

template<typename TIndex>
class graph_preamble;

template<typename TIndex, typename TWeight>
class graph_edge;

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph_edges(
  std::istream& is,
  I expected_vc,
  I expected_ec);

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  std::istream& is,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>);

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  std::istream& is,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>);

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  std::istream& is,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>);

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  std::istream& is,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>);

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  std::istream& is);

template<graph_format F, typename I, typename W>
void
print_graph_edges(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges);

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>);

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>);

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>);

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>);

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges);

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>);

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>);

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>);

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>);

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges);

} // namespace impl

//
// Forward declarations
// ---

template<typename TIndex>
class graph_preamble<graph_format::graph_fmt_dimacs, TIndex> : public impl::graph_preamble<TIndex>
{
public:
  graph_preamble()
    : impl::graph_preamble<TIndex>()
  {
  }

  graph_preamble(TIndex vertex_count, TIndex edge_count)
    : impl::graph_preamble<TIndex>(vertex_count, edge_count)
  {
  }

  graph_preamble(std::tuple<TIndex, TIndex> tuple)
    : impl::graph_preamble<TIndex>(std::get<0>(tuple), std::get<1>(tuple))
  {
  }

  friend std::istream&
  operator>><TIndex>(std::istream& is, graph_preamble<graph_format::graph_fmt_dimacs, TIndex>& preamble);

  friend std::ostream&
  operator<<<TIndex>(std::ostream& os, const graph_preamble<graph_format::graph_fmt_dimacs, TIndex>& preamble);
};

template<typename TIndex>
class graph_preamble<graph_format::graph_fmt_binary, TIndex> : public impl::graph_preamble<TIndex>
{
public:
  graph_preamble()
    : impl::graph_preamble<TIndex>()
  {
  }

  graph_preamble(TIndex vertex_count, TIndex edge_count)
    : impl::graph_preamble<TIndex>(vertex_count, edge_count)
  {
  }

  graph_preamble(std::tuple<TIndex, TIndex> tuple)
    : impl::graph_preamble<TIndex>(std::get<0>(tuple), std::get<1>(tuple))
  {
  }

  friend std::istream&
  operator>><TIndex>(std::istream& is, graph_preamble<graph_format::graph_fmt_binary, TIndex>& preamble);

  friend std::ostream&
  operator<<<TIndex>(std::ostream& os, const graph_preamble<graph_format::graph_fmt_binary, TIndex>& preamble);
};

template<typename TIndex, typename TWeight>
class graph_edge<graph_format::graph_fmt_edgelist, TIndex, TWeight> : public impl::graph_edge<TIndex, TWeight>
{
public:
  graph_edge()
    : impl::graph_edge<TIndex, TWeight>()
  {
  }

  graph_edge(TIndex from, TIndex to, TWeight weight)
    : impl::graph_edge<TIndex, TWeight>(from, to, weight)
  {
  }

  graph_edge(std::tuple<TIndex, TIndex, TWeight> tuple)
    : impl::graph_edge<TIndex, TWeight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple))
  {
  }

  friend std::istream&
  operator>><TIndex, TWeight>(std::istream& is, graph_edge<graph_format::graph_fmt_edgelist, TIndex, TWeight>& edge);

  friend std::ostream&
  operator<<<TIndex, TWeight>(std::ostream& os, const graph_edge<graph_format::graph_fmt_edgelist, TIndex, TWeight>& edge);
};

template<typename TIndex, typename TWeight>
class graph_edge<graph_format::graph_fmt_weightlist, TIndex, TWeight> : public impl::graph_edge<TIndex, TWeight>
{
public:
  graph_edge()
    : impl::graph_edge<TIndex, TWeight>()
  {
  }

  graph_edge(TIndex from, TIndex to, TWeight weight)
    : impl::graph_edge<TIndex, TWeight>(from, to, weight)
  {
  }

  graph_edge(std::tuple<TIndex, TIndex, TWeight> tuple)
    : impl::graph_edge<TIndex, TWeight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple))
  {
  }

  friend std::istream&
  operator>><TIndex, TWeight>(std::istream& is, graph_edge<graph_format::graph_fmt_weightlist, TIndex, TWeight>& edge);

  friend std::ostream&
  operator<<<TIndex, TWeight>(std::ostream& os, const graph_edge<graph_format::graph_fmt_weightlist, TIndex, TWeight>& edge);
};

template<typename TIndex, typename TWeight>
class graph_edge<graph_format::graph_fmt_dimacs, TIndex, TWeight> : public impl::graph_edge<TIndex, TWeight>
{
public:
  graph_edge()
    : impl::graph_edge<TIndex, TWeight>()
  {
  }

  graph_edge(TIndex from, TIndex to, TWeight weight)
    : impl::graph_edge<TIndex, TWeight>(from, to, weight)
  {
  }

  graph_edge(std::tuple<TIndex, TIndex, TWeight> tuple)
    : impl::graph_edge<TIndex, TWeight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple))
  {
  }

  friend std::istream&
  operator>><TIndex, TWeight>(std::istream& is, graph_edge<graph_format::graph_fmt_dimacs, TIndex, TWeight>& edge);

  friend std::ostream&
  operator<<<TIndex, TWeight>(std::ostream& os, const graph_edge<graph_format::graph_fmt_dimacs, TIndex, TWeight>& edge);
};

template<typename TIndex, typename TWeight>
class graph_edge<graph_format::graph_fmt_binary, TIndex, TWeight> : public impl::graph_edge<TIndex, TWeight>
{
public:
  graph_edge()
    : impl::graph_edge<TIndex, TWeight>()
  {
  }

  graph_edge(TIndex from, TIndex to, TWeight weight)
    : impl::graph_edge<TIndex, TWeight>(from, to, weight)
  {
  }

  graph_edge(std::tuple<TIndex, TIndex, TWeight> tuple)
    : impl::graph_edge<TIndex, TWeight>(std::get<0>(tuple), std::get<1>(tuple), std::get<2>(tuple))
  {
  }

  friend std::istream&
  operator>><TIndex, TWeight>(std::istream& is, graph_edge<graph_format::graph_fmt_binary, TIndex, TWeight>& edge);

  friend std::ostream&
  operator<<<TIndex, TWeight>(std::ostream& os, const graph_edge<graph_format::graph_fmt_binary, TIndex, TWeight>& edge);
};

template<typename TIndex>
std::istream&
operator>>(std::istream& is, graph_preamble<graph_format::graph_fmt_dimacs, TIndex>& preamble)
{
  return is;
};

template<typename TIndex>
std::istream&
operator>>(std::istream& is, graph_preamble<graph_format::graph_fmt_binary, TIndex>& preamble)
{
  return is;
};

template<typename TIndex>
std::ostream&
operator<<(std::ostream& os, const graph_preamble<graph_format::graph_fmt_dimacs, TIndex>& preamble)
{
  os << 'p' << ' ' << preamble.vertex_count() << ' ' << preamble.edge_count() << '\n';
  return os;
};

template<typename TIndex>
std::ostream&
operator<<(std::ostream& os, const graph_preamble<graph_format::graph_fmt_binary, TIndex>& preamble)
{
  TIndex v = preamble.vertex_count(), e = preamble.edge_count();

  os.write(reinterpret_cast<char*>(&v), sizeof(TIndex));
  os.write(reinterpret_cast<char*>(&e), sizeof(TIndex));

  return os;
};

template<typename TIndex, typename TWeight>
std::istream&
operator>>(std::istream& is, graph_edge<graph_format::graph_fmt_edgelist, TIndex, TWeight>& edge)
{
  std::string line;
  if (std::getline(is, line)) {
    TIndex f, t;

    std::stringstream ss(line);
    if (ss >> f >> t) {
      edge = graph_edge<graph_format::graph_fmt_edgelist, TIndex, TWeight>(f, t, TWeight(1));
      return is;
    }
  }

  is.setstate(std::ios::failbit);
  return is;
};

template<typename TIndex, typename TWeight>
std::istream&
operator>>(std::istream& is, graph_edge<graph_format::graph_fmt_weightlist, TIndex, TWeight>& edge)
{
  std::string line;
  if (std::getline(is, line)) {
    TIndex  f, t;
    TWeight w;

    std::stringstream ss(line);
    if (ss >> f >> t >> w) {
      edge = graph_edge<graph_format::graph_fmt_weightlist, TIndex, TWeight>(f, t, w);
      return is;
    }
  }

  is.setstate(std::ios::failbit);
  return is;
};

template<typename TIndex, typename TWeight>
std::istream&
operator>>(std::istream& is, graph_edge<graph_format::graph_fmt_dimacs, TIndex, TWeight>& edge)
{
  char        c;
  std::string v;

  for (;;) {
    if (!std::getline(is, v))
      return is;

    std::stringstream ss(v);
    if (!(ss >> c))
      return is;

    switch (c) {
      case 'c':
      case 'p':
        continue;
      case 'a': {
        TIndex  f, t;
        TWeight w;
        if (ss >> f) {
          if (ss >> t >> w) {
            edge = graph_edge<graph_format::graph_fmt_dimacs, TIndex, TWeight>(f, t, w);
          } else {
            is.setstate(std::ios::failbit);
          }
        }
        return is;
      }
      default:
        is.setstate(std::ios::failbit);
        return is;
    }
  }
  return is;
};

template<typename TIndex, typename TWeight>
std::istream&
operator>>(std::istream& is, graph_edge<graph_format::graph_fmt_binary, TIndex, TWeight>& edge)
{
  TIndex  f, t;
  TWeight w;

  if (!is.read(reinterpret_cast<char*>(&f), sizeof(TIndex)))
    return is;

  if (!is.read(reinterpret_cast<char*>(&t), sizeof(TIndex)))
    return is;

  if (!is.read(reinterpret_cast<char*>(&w), sizeof(TWeight)))
    return is;

  edge = graph_edge<graph_format::graph_fmt_binary, TIndex, TWeight>(f, t, w);

  return is;
};

template<typename TIndex, typename TWeight>
std::ostream&
operator<<(std::ostream& os, const graph_edge<graph_format::graph_fmt_edgelist, TIndex, TWeight>& edge)
{
  os << edge.from() << ' ' << edge.to() << '\n';
  return os;
};

template<typename TIndex, typename TWeight>
std::ostream&
operator<<(std::ostream& os, const graph_edge<graph_format::graph_fmt_weightlist, TIndex, TWeight>& edge)
{
  os << edge.from() << ' ' << edge.to() << ' ' << edge.weight() << '\n';
  return os;
};

template<typename TIndex, typename TWeight>
std::ostream&
operator<<(std::ostream& os, const graph_edge<graph_format::graph_fmt_dimacs, TIndex, TWeight>& edge)
{
  os << 'a' << ' ' << edge.from() << ' ' << edge.to() << ' ' << edge.weight() << '\n';
  return os;
};

template<typename TIndex, typename TWeight>
std::ostream&
operator<<(std::ostream& os, const graph_edge<graph_format::graph_fmt_binary, TIndex, TWeight>& edge)
{
  TIndex  f = edge.from(), t = edge.to();
  TWeight w = edge.weight();

  os.write(reinterpret_cast<char*>(&f), sizeof(TIndex));
  os.write(reinterpret_cast<char*>(&t), sizeof(TIndex));
  os.write(reinterpret_cast<char*>(&w), sizeof(TWeight));

  return os;
};

bool
parse_graph_format(
  const std::string& format,
  graph_format&      out_format)
{
  if (format == "edgelist") {
    out_format = graph_format::graph_fmt_edgelist;
    return true;
  }
  if (format == "weightlist") {
    out_format = graph_format::graph_fmt_weightlist;
    return true;
  }
  if (format == "dimacs") {
    out_format = graph_format::graph_fmt_dimacs;
    return true;
  }
  if (format == "binary") {
    out_format = graph_format::graph_fmt_binary;
    return true;
  }
  return false;
};

template<typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  graph_format  format,
  std::istream& is)
{
  switch (format) {
    case graph_format::graph_fmt_edgelist:
      return impl::scan_graph<graph_format::graph_fmt_edgelist, I, W>(is);
    case graph_format::graph_fmt_weightlist:
      return impl::scan_graph<graph_format::graph_fmt_weightlist, I, W>(is);
    case graph_format::graph_fmt_dimacs:
      return impl::scan_graph<graph_format::graph_fmt_dimacs, I, W>(is);
    case graph_format::graph_fmt_binary:
      return impl::scan_graph<graph_format::graph_fmt_binary, I, W>(is);
    default:
      throw std::logic_error("erro: The format is not supported");
  }
};

template<typename I, typename W>
void
print_graph(
  graph_format                      format,
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges)
{
  switch (format) {
    case graph_format::graph_fmt_edgelist:
      impl::print_graph<graph_format::graph_fmt_edgelist, I, W>(os, edges);
      break;
    case graph_format::graph_fmt_weightlist:
      impl::print_graph<graph_format::graph_fmt_weightlist, I, W>(os, edges);
      break;
    case graph_format::graph_fmt_dimacs:
      impl::print_graph<graph_format::graph_fmt_dimacs, I, W>(os, edges);
      break;
    case graph_format::graph_fmt_binary:
      impl::print_graph<graph_format::graph_fmt_binary, I, W>(os, edges);
      break;
    default:
      throw std::logic_error("erro: The format is not supported");
  }

  // Flush the output stream to ensure all of the graph content is in it
  //
  os.flush();
};

template<typename I, typename W>
void
print_graph(
  graph_format                      format,
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges)
{
  switch (format) {
    case graph_format::graph_fmt_edgelist:
      impl::print_graph<graph_format::graph_fmt_edgelist, I, W>(os, vc, edges);
      break;
    case graph_format::graph_fmt_weightlist:
      impl::print_graph<graph_format::graph_fmt_weightlist, I, W>(os, vc, edges);
      break;
    case graph_format::graph_fmt_dimacs:
      impl::print_graph<graph_format::graph_fmt_dimacs, I, W>(os, vc, edges);
      break;
    case graph_format::graph_fmt_binary:
      impl::print_graph<graph_format::graph_fmt_binary, I, W>(os, vc, edges);
      break;
    default:
      throw std::logic_error("erro: The format is not supported");
  }

  // Flush the output stream to ensure all of the graph content is in it
  //
  os.flush();
};

namespace impl {

template<typename TIndex>
class graph_preamble
{
private:
  TIndex m_vc;
  TIndex m_ec;

public:
  graph_preamble()
    : m_vc(TIndex(0))
    , m_ec(TIndex(0))
  {
  }

  graph_preamble(TIndex vertex_count, TIndex edge_count)
    : m_vc(vertex_count)
    , m_ec(edge_count)
  {
  }

  TIndex
  vertex_count() const
  {
    return this->m_vc;
  }

  TIndex
  edge_count() const
  {
    return this->m_ec;
  }
};

template<typename TIndex, typename TWeight>
class graph_edge
{
private:
  TIndex  m_f;
  TIndex  m_t;
  TWeight m_w;

public:
  graph_edge()
    : m_f(TIndex(0))
    , m_t(TIndex(0))
    , m_w(TWeight(0))
  {
  }

  graph_edge(TIndex from, TIndex to, TWeight weight)
    : m_f(from)
    , m_t(to)
    , m_w(weight)
  {
  }

  TIndex
  from() const
  {
    return this->m_f;
  }

  TIndex
  to() const
  {
    return this->m_t;
  }

  TWeight
  weight() const
  {
    return this->m_w;
  }
};

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph_edges(
  std::istream& is,
  I expected_vc,
  I expected_ec)
{
  I vmax = I(0), vc = I(0);

  std::vector<std::tuple<I, I, W>> edges;
  if (expected_ec != I(0))
    edges.reserve(expected_ec);

  io::graph_edge<F, I, W> edge;
  while (is >> edge) {
    edges.push_back(std::make_tuple(edge.from(), edge.to(), edge.weight()));

    vmax = std::max({ vmax, edge.from(), edge.to() });
  }
  vc = vmax + I(1);

  if (expected_vc != I(0) && expected_vc != vc)
    throw std::logic_error(
      "erro: the expected number of vertices (" + std::to_string(expected_vc)
        + ") don't match the number scanned ones (" + std::to_string(vc) + ")");

  if (expected_ec != I(0) && expected_ec != vc)
    throw std::logic_error(
      "erro: the expected number of edges (" + std::to_string(expected_ec)
        + ") don't match the number scanned ones (" + std::to_string(edges.size()) + ")");

  if (is.eof())
    return std::make_tuple(vc, edges);

  throw std::logic_error("erro: can't scan 'graph_edge' because of invalid format or IO problem");
};

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  std::istream& is,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>)
{
  return scan_graph_edges<F, I, W>(is, I(0), I(0));
};

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  std::istream& is,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>)
{
  io::graph_preamble<F, I> preamble;
  if (!(is >> preamble))
    throw std::logic_error("erro: can't scan 'graph_preamble' because of invalid format or IO problem");

  return scan_graph_edges<F, I, W>(is, preamble.vertex_count(), I(0));
};

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  std::istream& is,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>)
{
  io::graph_preamble<F, I> preamble;
  if (!(is >> preamble))
    throw std::logic_error("erro: can't scan 'graph_preamble' because of invalid format or IO problem");

  return scan_graph_edges<F, I, W>(is, I(0), preamble.edge_count());
};

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  std::istream& is,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>)
{
  io::graph_preamble<F, I> preamble;
  if (!(is >> preamble))
    throw std::logic_error("erro: can't scan 'graph_preamble' because of invalid format or IO problem");

  return scan_graph_edges<F, I, W>(is, preamble.vertex_count(), preamble.edge_count());
};

template<graph_format F, typename I, typename W>
std::tuple<I, std::vector<std::tuple<I, I, W>>>
scan_graph(
  std::istream& is)
{
  return scan_graph<F, I, W>(
    is,
    typename graph_traits<F>::preamble_format());
};

template<graph_format F, typename I, typename W>
void
print_graph_edges(
  std::ostream&                     os,
  I                                 expected_vc,
  std::vector<std::tuple<I, I, W>>& edges)
{
  I vmax = I(0), vc = I(0);

  for (auto item : edges) {
    auto [f, t, w] = item;

    io::graph_edge<F, I, W> edge(f, t, w);
    if (!(os << edge))
      throw std::logic_error("erro: can't print 'graph_edge' because of IO problem");

    vmax = std::max({ vmax, f, t });
  }
  vc = vmax + I(1);

  if (expected_vc != I(0) && expected_vc != vc)
    throw std::logic_error(
      "erro: the expected number of vertices (" + std::to_string(expected_vc)
        + ") don't match the number printed ones (" + std::to_string(vc) + ")");
};

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>)
{
  I vc = I(0), vmax = I(0);

  for (auto item : edges) {
    auto [f, t, _] = item;

    vmax = std::max({ vmax, f, t });
  }
  vc = vmax + I(1);

  io::graph_preamble<F, I> preamble(vc, edges.size());
  if (!(os << preamble))
    throw std::logic_error("erro: can't print 'graph_preamble' because of IO problem");

  print_graph_edges<F, I, W>(os, I(0), edges);
};

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>)
{
  print_graph<F, I, W>(
    os,
    edges,
    graph_preamble_format::graph_preamble_fmt_full);
};

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>)
{
  io::graph_preamble<F, I> preamble(I(0), edges.size());
  if (!(os << preamble))
    throw std::logic_error("erro: can't print 'graph_preamble' because of IO problem");

  print_graph_edges<F, I, W>(os, I(0), edges);
};

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>)
{
  print_graph_edges<F, I, W>(os, I(0), edges);
};

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  std::vector<std::tuple<I, I, W>>& edges)
{
  print_graph<F, I, W>(
    os,
    edges,
    typename graph_traits<F>::preamble_format());
};

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>)
{
  io::graph_preamble<F, I> preamble(vc, edges.size());
  if (!(os << preamble))
    throw std::logic_error("erro: can't print 'graph_preamble' because of IO problem");

  print_graph_edges<F, I, W>(os, vc, edges);
};

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>)
{
  print_graph<F, I, W>(
    os,
    vc,
    edges,
    graph_preamble_format::graph_preamble_fmt_full);
};

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>)
{
  io::graph_preamble<F, I> preamble(I(0), edges.size());
  if (!(os << preamble))
    throw std::logic_error("erro: can't print 'graph_preamble' because of IO problem");

  print_graph_edges<F, I, W>(os, vc, edges);
};

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>)
{
  print_graph_edges<F, I, W>(os, vc, edges);
};

template<graph_format F, typename I, typename W>
void
print_graph(
  std::ostream&                     os,
  I                                 vc,
  std::vector<std::tuple<I, I, W>>& edges)
{
  print_graph<F, I, W>(
    os,
    vc,
    edges,
    typename graph_traits<F>::preamble_format());
};

} // namespace impl

} // namespace io
} // namespace graphs
} // namespace utilz

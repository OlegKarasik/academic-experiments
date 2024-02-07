#pragma once

#include <functional>
#include <istream>
#include <memory>
#include <ostream>
#include <stdexcept>

namespace utilz {
namespace graphs {
namespace io {

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

namespace impl {

template<typename TIndex>
class graph_preamble;

template<typename TIndex, typename TWeight>
class graph_edge;

template<graph_format F, typename G, typename I, typename W, typename SW>
std::tuple<I, I>
scan_graph_edges(
  std::istream& is,
  G&            graph,
  SW&           set_w);

template<graph_format F, typename G, typename I, typename W, typename SV, typename SE, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SV&           set_vc,
  SE&           set_ec,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>);

template<graph_format F, typename G, typename I, typename W, typename SV, typename SE, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SV&           set_vc,
  SE&           set_ec,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>);

template<graph_format F, typename G, typename I, typename W, typename SV, typename SE, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SV&           set_vc,
  SE&           set_ec,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>);

template<graph_format F, typename G, typename I, typename W, typename SV, typename SE, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SV&           set_vc,
  SE&           set_ec,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>);

template<graph_format F, typename G, typename I, typename W, typename SV, typename SE, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SV&           set_vc,
  SE&           set_ec,
  SW&           set_w);

template<graph_format F, typename G, typename I, typename W, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>);

template<graph_format F, typename G, typename I, typename W, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>);

template<graph_format F, typename G, typename I, typename W, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>);

template<graph_format F, typename G, typename I, typename W, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>);

template<graph_format F, typename G, typename I, typename W, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SW&           set_w);

// ==

template<graph_format F, typename G, typename I, typename W, typename GW>
void
print_graph_edges(
  std::ostream& os,
  G&            graph,
  GW&           get_w);

template<graph_format F, typename G, typename I, typename W, typename GV, typename GE, typename GW>
void
print_graph(
  std::ostream& os,
  G&            graph,
  GV&           get_vc,
  GE&           get_ec,
  GW&           get_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>);

template<graph_format F, typename G, typename I, typename W, typename GV, typename GE, typename GW>
void
print_graph(
  std::ostream& os,
  G&            graph,
  GV&           get_vc,
  GE&           get_ec,
  GW&           get_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>);

template<graph_format F, typename G, typename I, typename W, typename GV, typename GE, typename GW>
void
print_graph(
  std::ostream& os,
  G&            graph,
  GV&           get_vc,
  GE&           get_ec,
  GW&           get_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>);

template<graph_format F, typename G, typename I, typename W, typename GV, typename GE, typename GW>
void
print_graph(
  std::ostream& os,
  G&            graph,
  GV&           get_vc,
  GE&           get_ec,
  GW&           get_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>);

template<graph_format F, typename G, typename I, typename W, typename GV, typename GE, typename GW>
void
print_graph(
  std::ostream& os,
  G&            graph,
  GV&           get_vc,
  GE&           get_ec,
  GW&           get_w);

} // namespace impl

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
  TIndex f, t;
  if (is >> f >> t) {
    edge = graph_edge<graph_format::graph_fmt_edgelist, TIndex, TWeight>(f, t, TWeight(1));
  }
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

//
// Forward declarations
// ---

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

template<typename G, typename I, typename W>
void
scan_graph(
  graph_format                      format,
  std::istream&                     is,
  G&                                graph,
  std::function<void(G&, I)>&       set_vc,
  std::function<void(G&, I)>&       set_ec,
  std::function<void(G&, I, I, W)>& set_w)
{
  using SE = typename std::function<void(G&, I)>;
  using SV = typename std::function<void(G&, I)>;
  using SW = typename std::function<void(G&, I, I, W)>;

  switch (format) {
    case graph_format::graph_fmt_edgelist:
      impl::scan_graph<graph_format::graph_fmt_edgelist, G, I, W, SV, SE, SW>(is, graph, set_vc, set_ec, set_w);
      break;
    case graph_format::graph_fmt_weightlist:
      impl::scan_graph<graph_format::graph_fmt_weightlist, G, I, W, SV, SE, SW>(is, graph, set_vc, set_ec, set_w);
      break;
    case graph_format::graph_fmt_dimacs:
      impl::scan_graph<graph_format::graph_fmt_dimacs, G, I, W, SV, SE, SW>(is, graph, set_vc, set_ec, set_w);
      break;
    case graph_format::graph_fmt_binary:
      impl::scan_graph<graph_format::graph_fmt_binary, G, I, W, SV, SE, SW>(is, graph, set_vc, set_ec, set_w);
      break;
    default:
      throw std::logic_error("erro: The format is not supported");
  }
};

template<typename G, typename I, typename W>
void
scan_graph(
  graph_format                      format,
  std::istream&                     is,
  G&                                graph,
  std::function<void(G&, I, I, W)>& set_w)
{
  using SW = typename std::function<void(G&, I, I, W)>;

  switch (format) {
    case graph_format::graph_fmt_edgelist:
      impl::scan_graph<graph_format::graph_fmt_edgelist, G, I, W, SW>(is, graph, set_w);
      break;
    case graph_format::graph_fmt_weightlist:
      impl::scan_graph<graph_format::graph_fmt_weightlist, G, I, W, SW>(is, graph, set_w);
      break;
    case graph_format::graph_fmt_dimacs:
      impl::scan_graph<graph_format::graph_fmt_dimacs, G, I, W, SW>(is, graph, set_w);
      break;
    case graph_format::graph_fmt_binary:
      impl::scan_graph<graph_format::graph_fmt_binary, G, I, W, SW>(is, graph, set_w);
      break;
    default:
      throw std::logic_error("erro: The format is not supported");
  }
};

template<typename G, typename I, typename W>
void
pring_graph(
  graph_format                format,
  std::ostream&               os,
  G&                          graph,
  std::function<I(G&)>&       get_vc,
  std::function<I(G&)>&       get_ec,
  std::function<W(G&, I, I)>& get_w)
{
  using GE = typename std::function<I(G&)>;
  using GV = typename std::function<I(G&)>;
  using GW = typename std::function<W(G&, I, I)>;

  // switch (format) {
  //   case graph_format::graph_fmt_edgelist:
  //     impl::scan_graph<graph_format::graph_fmt_edgelist, G, I, W, SV, SE, SW>(is, graph, set_vc, set_ec, set_w);
  //     break;
  //   case graph_format::graph_fmt_weightlist:
  //     impl::scan_graph<graph_format::graph_fmt_weightlist, G, I, W, SV, SE, SW>(is, graph, set_vc, set_ec, set_w);
  //     break;
  //   case graph_format::graph_fmt_dimacs:
  //     impl::scan_graph<graph_format::graph_fmt_dimacs, G, I, W, SV, SE, SW>(is, graph, set_vc, set_ec, set_w);
  //     break;
  //   case graph_format::graph_fmt_binary:
  //     impl::scan_graph<graph_format::graph_fmt_binary, G, I, W, SV, SE, SW>(is, graph, set_vc, set_ec, set_w);
  //     break;
  //   default:
  //     throw std::logic_error("erro: The format is not supported");
  // }
};

template<typename G, typename I, typename W>
void
pring_graph(
  graph_format                format,
  std::ostream&               os,
  G&                          graph,
  std::function<W(G&, I, I)>& get_w)
{
  using GW = typename std::function<W(G&, I, I)>;

  // switch (format) {
  //   case graph_format::graph_fmt_edgelist:
  //     impl::scan_graph<graph_format::graph_fmt_edgelist, G, I, W, SW>(is, graph, set_w);
  //     break;
  //   case graph_format::graph_fmt_weightlist:
  //     impl::scan_graph<graph_format::graph_fmt_weightlist, G, I, W, SW>(is, graph, set_w);
  //     break;
  //   case graph_format::graph_fmt_dimacs:
  //     impl::scan_graph<graph_format::graph_fmt_dimacs, G, I, W, SW>(is, graph, set_w);
  //     break;
  //   case graph_format::graph_fmt_binary:
  //     impl::scan_graph<graph_format::graph_fmt_binary, G, I, W, SW>(is, graph, set_w);
  //     break;
  //   default:
  //     throw std::logic_error("erro: The format is not supported");
  // }
};
/*
template<typename Graph, typename GraphGetVertexOperation, typename GraphGetEdgeCountOperation, typename GraphGetValueOperation>
void
print_graph(
  std::ostream&               s,
  graph_stream_format         format,
  Graph&                      g,
  GraphGetVertexOperation&    get_vertex_count,
  GraphGetEdgeCountOperation& get_edge_count,
  GraphGetValueOperation&     get_value)
{
  using size_type  = typename Graph::size_type;
  using value_type = typename Graph::value_type;

  auto gdetails = get_graph_stream_format_details(format);
  auto gostream = make_graph_ostream<size_type, value_type>(s, format);
  auto gos      = gostream.get();

  size_type vertex_count = get_vertex_count(g);
  size_type edge_count   = get_edge_count(g);

  if (vertex_count == size_type(0))
    throw std::logic_error("erro: 'get_vertex_count()' must return vertex count");

  if (gdetails.preamble_required()) {
    if (gdetails.preamble_includes_edge_count() && edge_count == size_type(0)) {
      for (size_type i = size_type(0); i < vertex_count; ++i)
        for (size_type j = size_type(0); j < vertex_count; ++j) {
          value_type v = get_value(g, i, j);
          if (v != value_type(0))
            ++edge_count;
        }
    }

    graph_preamble<size_type> preamble(vertex_count, edge_count);
    if (!(gos << preamble))
      throw std::logic_error("erro: can't print 'graph_preamble' because of IO problem");
  }

  for (size_type i = size_type(0); i < vertex_count; ++i)
    for (size_type j = size_type(0); j < vertex_count; ++j) {
      value_type v = get_value(g, i, j);
      if (v != value_type(0)) {
        graph_edge<size_type, value_type> edge(i, j, v);
        if (!(gos << edge))
          throw std::logic_error("erro: can't print 'graph_edge' because of IO problem");
      }
    }

  s.flush();
};
*/

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

template<graph_format F, typename G, typename I, typename W, typename SW>
std::tuple<I, I>
scan_graph_edges(
  std::istream& is,
  G&            graph,
  SW&           set_w)
{
  I vmin = I(0), vmax = I(0), vc = I(0), ec = I(0);

  io::graph_edge<F, I, W> edge;
  while (is >> edge) {
    set_w(graph, edge.from(), edge.to(), edge.weight());

    vmin = std::min({ vmin, edge.from(), edge.to() });
    vmax = std::max({ vmax, edge.from(), edge.to() });

    ec++;
  }
  vc = vmin == I(0) ? vmax + I(1) : vmax;

  if (is.eof())
    return std::make_tuple(vc, ec);

  throw std::logic_error("erro: can't scan 'graph_edge' because of invalid format or IO problem");
};

template<graph_format F, typename G, typename I, typename W, typename SV, typename SE, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SV&           set_vc,
  SE&           set_ec,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>)
{
  using CACHE_SW = typename std::function<void(std::vector<io::graph_edge<F, I, W>>&, I, I, W)>;
  using CACHE_G  = typename std::vector<io::graph_edge<F, I, W>>;

  auto cache_w = std::function([](std::vector<io::graph_edge<F, I, W>>& v, I f, I t, W w) -> void {
    v.push_back(io::graph_edge<F, I, W>(f, t, w));
  });

  std::vector<io::graph_edge<F, I, W>> cache;

  I vc, ec;
  std::tie(vc, ec) = scan_graph_edges<F, CACHE_G, I, W, CACHE_SW>(is, cache, cache_w);

  set_vc(graph, vc);
  set_ec(graph, ec);

  for (auto e : cache)
    set_w(graph, e.from(), e.to(), e.weight());
};

template<graph_format F, typename G, typename I, typename W, typename SV, typename SE, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SV&           set_vc,
  SE&           set_ec,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>)
{
  scan_graph<F, G, I, W, SV, SE, SW>(
    is,
    graph,
    set_vc,
    set_ec,
    set_w,
    std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>());
};

template<graph_format F, typename G, typename I, typename W, typename SV, typename SE, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SV&           set_vc,
  SE&           set_ec,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>)
{
  scan_graph<F, G, I, W, SV, SE, SW>(
    is,
    graph,
    set_vc,
    set_ec,
    set_w,
    std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>());
};

template<graph_format F, typename G, typename I, typename W, typename SV, typename SE, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SV&           set_vc,
  SE&           set_ec,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>)
{
  io::graph_preamble<F, I> preamble;
  if (!(is >> preamble))
    throw std::logic_error("erro: can't scan 'graph_preamble' because of invalid format or IO problem");

  set_vc(graph, preamble.vertex_count());
  set_ec(graph, preamble.edge_count());

  scan_graph_edges<F, G, I, W, SW>(is, graph, set_w);
};

template<graph_format F, typename G, typename I, typename W, typename SV, typename SE, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SV&           set_vc,
  SE&           set_ec,
  SW&           set_w)
{
  scan_graph<F, G, I, W, SV, SE, SW>(
    is,
    graph,
    set_vc,
    set_ec,
    set_w,
    typename graph_traits<F>::preamble_format());
};

template<graph_format F, typename G, typename I, typename W, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>)
{
  io::graph_preamble<F, I> preamble;
  if (!(is >> preamble))
    throw std::logic_error("erro: can't scan 'graph_preamble' because of invalid format or IO problem");

  scan_graph_edges<F, G, I, W, SW>(is, graph, set_w);
};

template<graph_format F, typename G, typename I, typename W, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>)
{
  scan_graph<F, G, I, W, SW>(
    is,
    graph,
    set_w,
    std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>());
};

template<graph_format F, typename G, typename I, typename W, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>)
{
  scan_graph<F, G, I, W, SW>(
    is,
    graph,
    set_w,
    std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>());
};

template<graph_format F, typename G, typename I, typename W, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SW&           set_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>)
{
  scan_graph_edges<F, G, I, W, SW>(is, graph, set_w);
};

template<graph_format F, typename G, typename I, typename W, typename SW>
void
scan_graph(
  std::istream& is,
  G&            graph,
  SW&           set_w)
{
  scan_graph<F, G, I, W, SW>(is, graph, set_w, typename graph_traits<F>::preamble_format());
};

template<graph_format F, typename G, typename I, typename W, typename GW>
void
print_graph_edges(
  std::ostream& os,
  G&            graph,
  GW&           get_w)
{

};

template<graph_format F, typename G, typename I, typename W, typename GV, typename GE, typename GW>
void
print_graph(
  std::ostream& os,
  G&            graph,
  GV&           get_vc,
  GE&           get_ec,
  GW&           get_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_none>)
{

};

template<graph_format F, typename G, typename I, typename W, typename GV, typename GE, typename GW>
void
print_graph(
  std::ostream& os,
  G&            graph,
  GV&           get_vc,
  GE&           get_ec,
  GW&           get_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_vertex_count>)
{

};

template<graph_format F, typename G, typename I, typename W, typename GV, typename GE, typename GW>
void
print_graph(
  std::ostream& os,
  G&            graph,
  GV&           get_vc,
  GE&           get_ec,
  GW&           get_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_edge_count>)
{

};

template<graph_format F, typename G, typename I, typename W, typename GV, typename GE, typename GW>
void
print_graph(
  std::ostream& os,
  G&            graph,
  GV&           get_vc,
  GE&           get_ec,
  GW&           get_w,
  std::integral_constant<graph_preamble_format, graph_preamble_format::graph_preamble_fmt_full>)
{

};

template<graph_format F, typename G, typename I, typename W, typename GV, typename GE, typename GW>
void
print_graph(
  std::ostream& os,
  G&            graph,
  GV&           get_vc,
  GE&           get_ec,
  GW&           get_w)
{
  print_graph<F, G, I, W, GV, GE, GW>(
    os,
    graph,
    get_vc,
    get_ec,
    get_w,
    typename graph_traits<F>::preamble_format());
};

} // namespace impl

} // namespace io
} // namespace graphs
} // namespace utilz

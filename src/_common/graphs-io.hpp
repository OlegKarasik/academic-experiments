#pragma once

#include <istream>
#include <memory>
#include <ostream>
#include <stdexcept>

namespace utilz {
namespace graphs {
namespace io {

enum graph_stream_format
{
  fmt_none,
  fmt_edgelist,
  fmt_weightlist,
  fmt_dimacs,
  fmt_binary
};

enum graph_stream_format_preamble
{
  fmt_preamble_none         = 0,
  fmt_preamble_vertex_count = 1,
  fmt_preamble_edge_count   = 2,
  fmt_preamble_full         = 3
};

bool
parse_graph_stream_format(const std::string& format, utilz::graphs::io::graph_stream_format& out_format)
{
  if (format == "edgelist") {
    out_format = utilz::graphs::io::graph_stream_format::fmt_edgelist;
    return true;
  }
  if (format == "weightlist") {
    out_format = utilz::graphs::io::graph_stream_format::fmt_weightlist;
    return true;
  }
  if (format == "dimacs") {
    out_format = utilz::graphs::io::graph_stream_format::fmt_dimacs;
    return true;
  }
  if (format == "binary") {
    out_format = utilz::graphs::io::graph_stream_format::fmt_binary;
    return true;
  }
  return false;
};

class graph_stream_format_details
{
private:
  graph_stream_format_preamble m_preamble;

public:
  graph_stream_format_details(graph_stream_format_preamble preamble)
    : m_preamble(preamble)
  {
  }

  bool
  preamble_required() const
  {
    return this->m_preamble != 0;
  }

  bool
  preamble_includes_vertex_count() const
  {
    return this->m_preamble & graph_stream_format_preamble::fmt_preamble_vertex_count;
  }

  bool
  preamble_includes_edge_count() const
  {
    return this->m_preamble & graph_stream_format_preamble::fmt_preamble_edge_count;
  }
};

graph_stream_format_details
get_graph_stream_format_details(utilz::graphs::io::graph_stream_format format)
{
  switch (format) {
    case graph_stream_format::fmt_edgelist:
      return graph_stream_format_details(graph_stream_format_preamble::fmt_preamble_none);
    case graph_stream_format::fmt_weightlist:
      return graph_stream_format_details(graph_stream_format_preamble::fmt_preamble_none);
    case graph_stream_format::fmt_dimacs:
      return graph_stream_format_details(graph_stream_format_preamble::fmt_preamble_full);
    case graph_stream_format::fmt_binary:
      return graph_stream_format_details(graph_stream_format_preamble::fmt_preamble_vertex_count);
  };
  throw std::logic_error("erro: unknown graph format");
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
class graph_istream;

template<typename TIndex, typename TWeight>
bool
operator>>(graph_istream<TIndex, TWeight>* s, graph_edge<TIndex, TWeight>& edge);

template<typename TIndex, typename TWeight>
bool
operator>>(graph_istream<TIndex, TWeight>* s, graph_preamble<TIndex>& preamble);

template<typename TIndex, typename TWeight>
class graph_istream
{
private:
  std::istream& m_s;

protected:
  virtual void
  read_edge(std::istream& s, graph_edge<TIndex, TWeight>& edge) = 0;

  virtual void
  read_preamble(std::istream& s, graph_preamble<TIndex>& preamble) = 0;

public:
  graph_istream(std::istream& s)
    : m_s(s){};

  bool
  eof() const
  {
    return this->m_s.eof();
  }

  bool
  fail() const
  {
    return this->m_s.fail();
  }

  bool
  good() const
  {
    return this->m_s.good();
  }

  operator bool() const
  {
    return this->m_s.good();
  }

  friend bool
  operator>><TIndex, TWeight>(graph_istream<TIndex, TWeight>* is, graph_edge<TIndex, TWeight>& edge);

  friend bool
  operator>><TIndex, TWeight>(graph_istream<TIndex, TWeight>* is, graph_preamble<TIndex>& preamble);
};

template<typename TIndex, typename TWeight>
bool
operator>>(graph_istream<TIndex, TWeight>* s, graph_edge<TIndex, TWeight>& edge)
{
  s->read_edge(s->m_s, edge);

  return s->m_s.good();
};

template<typename TIndex, typename TWeight>
bool
operator>>(graph_istream<TIndex, TWeight>* s, graph_preamble<TIndex>& preamble)
{
  s->read_preamble(s->m_s, preamble);

  return s->m_s.good();
};

template<typename TIndex, typename TWeight>
class graph_ostream;

template<typename TIndex, typename TWeight>
bool
operator<<(graph_ostream<TIndex, TWeight>* s, graph_edge<TIndex, TWeight>& edge);

template<typename TIndex, typename TWeight>
bool
operator<<(graph_ostream<TIndex, TWeight>* s, graph_preamble<TIndex>& preamble);

template<typename TIndex, typename TWeight>
class graph_ostream
{
private:
  std::ostream& m_s;

protected:
  virtual void
  write_edge(std::ostream& s, graph_edge<TIndex, TWeight>& edge) = 0;

  virtual void
  write_preamble(std::ostream& s, graph_preamble<TIndex>& preamble) = 0;

public:
  graph_ostream(std::ostream& s)
    : m_s(s){};

  bool
  fail() const
  {
    return this->m_s.fail();
  }

  bool
  good() const
  {
    return this->m_s.good();
  }

  operator bool() const
  {
    return this->m_s.good();
  }

  friend bool
  operator<<<TIndex, TWeight>(graph_ostream<TIndex, TWeight>* s, graph_edge<TIndex, TWeight>& edge);

  friend bool
  operator<<<TIndex, TWeight>(graph_ostream<TIndex, TWeight>* s, graph_preamble<TIndex>& preamble);
};

template<typename TIndex, typename TWeight>
bool
operator<<(graph_ostream<TIndex, TWeight>* s, graph_edge<TIndex, TWeight>& edge)
{
  s->write_edge(s->m_s, edge);

  return s->m_s.good();
};

template<typename TIndex, typename TWeight>
bool
operator<<(graph_ostream<TIndex, TWeight>* s, graph_preamble<TIndex>& preamble)
{
  s->write_preamble(s->m_s, preamble);

  return s->m_s.good();
};

template<typename TIndex, typename TWeight>
class graph_istream_edgelist : public graph_istream<TIndex, TWeight>
{
private:
  std::string m_pline;

protected:
  void
  read_edge(std::istream& s, graph_edge<TIndex, TWeight>& edge) override
  {
    TIndex f, t;
    if (s >> f) {
      if (s >> t) {
        edge = graph_edge<TIndex, TWeight>(f, t, TWeight(1));
      } else {
        s.setstate(std::ios::failbit);
      }
    }
  }

  void
  read_preamble(std::istream& s, graph_preamble<TIndex>& preamble) override
  {
  }

public:
  graph_istream_edgelist(std::istream& s)
    : graph_istream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
class graph_istream_dimacs : public graph_istream<TIndex, TWeight>
{
protected:
  void
  read_edge(std::istream& s, graph_edge<TIndex, TWeight>& edge) override
  {
    char        c;
    std::string v;

    for (;;) {
      if (!std::getline(s, v))
        return;

      std::stringstream ss(v);
      if (!(ss >> c))
        return;

      switch (c) {
        case 'c':
        case 'p':
          continue;
        case 'a': {
          TIndex  f, t;
          TWeight w;
          if (s >> f) {
            if (s >> t >> w) {
              edge = graph_edge<TIndex, TWeight>(f, t, w);
            } else {
              s.setstate(std::ios::failbit);
            }
          }
          return;
        }
        default:
          s.setstate(std::ios::failbit);
          return;
      }
    }
  }

  void
  read_preamble(std::istream& s, graph_preamble<TIndex>& preamble) override
  {
  }

public:
  graph_istream_dimacs(std::istream& s)
    : graph_istream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
class graph_istream_weightlist : public graph_istream<TIndex, TWeight>
{
protected:
  void
  read_edge(std::istream& s, graph_edge<TIndex, TWeight>& edge) override
  {
    TIndex  f, t;
    TWeight w;
    if (s >> f) {
      if (s >> t >> w) {
        edge = graph_edge<TIndex, TWeight>(f, t, w);
      } else {
        s.setstate(std::ios::failbit);
      }
    }
  }

  void
  read_preamble(std::istream& s, graph_preamble<TIndex>& preamble) override
  {
  }

public:
  graph_istream_weightlist(std::istream& s)
    : graph_istream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
class graph_istream_binary : public graph_istream<TIndex, TWeight>
{
protected:
  void
  read_edge(std::istream& s, graph_edge<TIndex, TWeight>& edge) override
  {
    TIndex  f, t;
    TWeight w;

    if (!s.read(reinterpret_cast<char*>(&f), sizeof(TIndex)))
      return;

    if (!s.read(reinterpret_cast<char*>(&t), sizeof(TIndex)))
      return;

    if (!s.read(reinterpret_cast<char*>(&w), sizeof(TWeight)))
      return;

    edge = graph_edge<TIndex, TWeight>(f, t, w);
  }

  void
  read_preamble(std::istream& s, graph_preamble<TIndex>& preamble) override
  {
  }

public:
  graph_istream_binary(std::istream& s)
    : graph_istream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
class graph_ostream_edgelist : public graph_ostream<TIndex, TWeight>
{
protected:
  void
  write_edge(std::ostream& s, graph_edge<TIndex, TWeight>& edge) override
  {
    s << edge.from() << ' ' << edge.to() << '\n';
  }

  void
  write_preamble(std::ostream& s, graph_preamble<TIndex>& preamble) override
  {
    s << preamble.vertex_count() << '\n';
  }

public:
  graph_ostream_edgelist(std::ostream& s)
    : graph_ostream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
class graph_ostream_dimacs : public graph_ostream<TIndex, TWeight>
{
protected:
  void
  write_edge(std::ostream& s, graph_edge<TIndex, TWeight>& edge) override
  {
    s << 'a' << ' ' << edge.from() << ' ' << edge.to() << ' ' << edge.weight() << '\n';
  }

  void
  write_preamble(std::ostream& s, graph_preamble<TIndex>& preamble) override
  {
    s << 'p' << ' ' << preamble.vertex_count() << ' ' << preamble.edge_count() << '\n';
  }

public:
  graph_ostream_dimacs(std::ostream& s)
    : graph_ostream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
class graph_ostream_weightlist : public graph_ostream<TIndex, TWeight>
{
protected:
  void
  write_edge(std::ostream& s, graph_edge<TIndex, TWeight>& edge) override
  {
    s << edge.from() << ' ' << edge.to() << ' ' << edge.weight() << '\n';
  }

  void
  write_preamble(std::ostream& s, graph_preamble<TIndex>& preamble) override
  {
    s << 'p' << ' ' << preamble.vertex_count() << '\n';
  }

public:
  graph_ostream_weightlist(std::ostream& s)
    : graph_ostream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
class graph_ostream_binary : public graph_ostream<TIndex, TWeight>
{
protected:
  void
  write_edge(std::ostream& s, graph_edge<TIndex, TWeight>& edge) override
  {
    TIndex  f = edge.from(), t = edge.to();
    TWeight w = edge.weight();

    if (!s.write(reinterpret_cast<char*>(&f), sizeof(TIndex)))
      return;

    if (!s.write(reinterpret_cast<char*>(&t), sizeof(TIndex)))
      return;

    if (!s.write(reinterpret_cast<char*>(&w), sizeof(TWeight)))
      return;
  }

  void
  write_preamble(std::ostream& s, graph_preamble<TIndex>& preamble) override
  {
    TIndex f = preamble.vertex_count();

    if (!s.write(reinterpret_cast<char*>(&f), sizeof(TIndex)))
      return;
  }

public:
  graph_ostream_binary(std::ostream& s)
    : graph_ostream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
std::unique_ptr<graph_istream<TIndex, TWeight>>
make_graph_istream(std::istream& s, graph_stream_format format)
{
  switch (format) {
    case graph_stream_format::fmt_edgelist:
      return std::unique_ptr<graph_istream<TIndex, TWeight>>(new graph_istream_edgelist<TIndex, TWeight>(s));
    case graph_stream_format::fmt_weightlist:
      return std::unique_ptr<graph_istream<TIndex, TWeight>>(new graph_istream_weightlist<TIndex, TWeight>(s));
    case graph_stream_format::fmt_dimacs:
      return std::unique_ptr<graph_istream<TIndex, TWeight>>(new graph_istream_dimacs<TIndex, TWeight>(s));
    case graph_stream_format::fmt_binary:
      return std::unique_ptr<graph_istream<TIndex, TWeight>>(new graph_istream_binary<TIndex, TWeight>(s));
  };
  throw std::logic_error("erro: unknown graph format");
};

template<typename TIndex, typename TWeight>
std::unique_ptr<graph_ostream<TIndex, TWeight>>
make_graph_ostream(std::ostream& s, graph_stream_format format)
{
  switch (format) {
    case graph_stream_format::fmt_edgelist:
      return std::unique_ptr<graph_ostream<TIndex, TWeight>>(new graph_ostream_edgelist<TIndex, TWeight>(s));
    case graph_stream_format::fmt_dimacs:
      return std::unique_ptr<graph_ostream<TIndex, TWeight>>(new graph_ostream_dimacs<TIndex, TWeight>(s));
    case graph_stream_format::fmt_weightlist:
      return std::unique_ptr<graph_ostream<TIndex, TWeight>>(new graph_ostream_weightlist<TIndex, TWeight>(s));
    case graph_stream_format::fmt_binary:
      return std::unique_ptr<graph_ostream<TIndex, TWeight>>(new graph_ostream_binary<TIndex, TWeight>(s));
  };
  throw std::logic_error("erro: unknown graph format");
};

template<typename G>
struct null_set_function
{
public:
  void
  operator()(G&, typename G::size_type&)
  {
  }
};

template<typename G>
struct null_get_function
{
public:
  typename G::size_type
  operator()(G&)
  {
    return typename G::size_type(0);
  }
};

template<typename G>
struct value_set_function
{
private:
  typename G::size_type m_value;

public:
  typename G::size_type
  value()
  {
    return this->m_value;
  }

  void
  operator()(G&, typename G::size_type& value)
  {
    this->m_value = value;
  }
};

template<typename G>
struct value_get_function
{
private:
  typename G::size_type m_value;

public:
  value_get_function(typename G::size_type& value)
    : m_value(value)
  {
  }

  typename G::size_type
  operator()(G&)
  {
    return this->m_value;
  }
};

template<typename G>
struct value_move_function
{
private:
  typename G::size_type m_value;

public:
  void
  operator()(G&, typename G::size_type& value)
  {
    this->m_value = value;
  }

  typename G::size_type
  operator()(G&)
  {
    return this->m_value;
  }
};

template<typename Graph, typename GraphSetVertexCountOperation, typename GraphSetEdgeCountOperation, typename GraphSetValueOperation>
void
scan_graph(
  std::istream&                 s,
  graph_stream_format           format,
  Graph&                        g,
  GraphSetVertexCountOperation& set_vertex_count,
  GraphSetEdgeCountOperation&   set_edge_count,
  GraphSetValueOperation&       set_value)
{
  using size_type  = typename Graph::size_type;
  using value_type = typename Graph::value_type;

  auto gdetails = get_graph_stream_format_details(format);
  auto gistream = make_graph_istream<size_type, value_type>(s, format);
  auto gis      = gistream.get();

  graph_preamble<size_type> preamble;
  if (gdetails.preamble_required()) {
    if (!(gis >> preamble))
      throw std::logic_error("erro: can't scan 'graph_preamble' because of invalid format or IO problem");
  }

  if (!gdetails.preamble_includes_vertex_count() || !gdetails.preamble_includes_edge_count()) {
    // Read all of the data into temporary buffer of edges
    // and calculate number of vertex at the same time
    //
    std::vector<graph_edge<size_type, value_type>> edges;

    size_type vmin = size_type(0),
              vmax = size_type(0);

    graph_edge<size_type, value_type> edge;
    while (gis >> edge) {
      edges.push_back(edge);

      vmin = std::min({ vmin, edge.from(), edge.to() });
      vmax = std::max({ vmax, edge.from(), edge.to() });
    }
    if (gis->fail())
      throw std::logic_error("erro: input is invalid or incomplete");

    size_type vertex_count = vmin == size_type(0) ? vmax + size_type(1) : vmax,
              edge_count   = size_type(edges.size());

    set_vertex_count(g, vertex_count);
    set_edge_count(g, edge_count);

    for (auto edge : edges)
      set_value(g, edge.from(), edge.to(), edge.weight());
  } else {
    // Resize graph
    //
    size_type vertex_count = preamble.vertex_count(),
              edge_count   = preamble.edge_count();

    set_vertex_count(g, vertex_count);
    set_edge_count(g, edge_count);

    // Keep reading edges (`from vertex` `to vertex` `the weight`)
    // as many as possible
    //
    graph_edge<size_type, value_type> edge;
    while (gis >> edge)
      set_value(g, edge.from(), edge.to(), edge.weight());

    if (gis->fail())
      throw std::logic_error("erro: input is invalid or incomplete");
  }
};

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

} // namespace io
} // namespace graphs
} // namespace utilz

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
  none,
  edgelist,
  weightlist,
  dimacs
};

bool
parse_graph_stream_format(const std::string& format, utilz::graphs::io::graph_stream_format& out_format)
{
  if (format == "edgelist") {
    out_format = utilz::graphs::io::graph_stream_format::edgelist;
    return true;
  }
  if (format == "weightlist") {
    out_format = utilz::graphs::io::graph_stream_format::weightlist;
    return true;
  }
  if (format == "dimacs") {
    out_format = utilz::graphs::io::graph_stream_format::dimacs;
    return true;
  }
  return false;
};

template<typename TIndex, typename TWeight>
class graph_istream;

template<typename TIndex, typename TWeight>
bool
operator>>(graph_istream<TIndex, TWeight>* is, std::tuple<TIndex, TIndex, TWeight>& obj);

template<typename TIndex, typename TWeight>
class graph_istream
{
private:
  std::istream& m_s;

  bool m_eof;
  bool m_err;

protected:
  virtual bool
  read_edge(std::istream& s, std::tuple<TIndex, TIndex, TWeight>& obj) = 0;

public:
  graph_istream(std::istream& s)
    : m_s(s)
    , m_eof(s.eof())
    , m_err(s.fail()){};

  bool
  eof()
  {
    return this->m_eof;
  };

  bool
  err()
  {
    return this->m_err;
  };

  operator bool() const
  {
    return this->m_eof || this->m_err;
  }

  friend bool
  operator>><TIndex, TWeight>(graph_istream<TIndex, TWeight>* is, std::tuple<TIndex, TIndex, TWeight>& obj);
};

template<typename TIndex, typename TWeight>
bool
operator>>(graph_istream<TIndex, TWeight>* s, std::tuple<TIndex, TIndex, TWeight>& obj)
{
  if (!s->read_edge(s->m_s, obj))
    s->m_s.setstate(std::ios::failbit);

  return s->m_s.good();
};

template<typename TIndex, typename TWeight>
class graph_ostream;

template<typename TIndex, typename TWeight>
bool
operator<<(graph_ostream<TIndex, TWeight>* is, std::tuple<TIndex, TIndex, TWeight>& obj);

template<typename TIndex, typename TWeight>
class graph_ostream
{
private:
  std::ostream& m_s;

  bool m_err;

protected:
  virtual bool
  write_edge(std::ostream& s, std::tuple<TIndex, TIndex, TWeight>& obj) = 0;

public:
  graph_ostream(std::ostream& s)
    : m_s(s)
    , m_err(s.fail()){};

  bool
  err()
  {
    return this->m_err;
  };

  operator bool() const
  {
    return this->m_err;
  }

  friend bool
  operator<<<TIndex, TWeight>(graph_ostream<TIndex, TWeight>* os, std::tuple<TIndex, TIndex, TWeight>& obj);
};

template<typename TIndex, typename TWeight>
bool
operator<<(graph_ostream<TIndex, TWeight>* s, std::tuple<TIndex, TIndex, TWeight>& obj)
{
  if (!s->write_edge(s->m_s, obj))
    s->m_s.setstate(std::ios::failbit);

  return s->m_s.good();
};

template<typename TIndex, typename TWeight>
class graph_istream_edgelist : public graph_istream<TIndex, TWeight>
{
protected:
  bool
  read_edge(std::istream& s, std::tuple<TIndex, TIndex, TWeight>& obj) override
  {
    TIndex f, t;
    if (!(s >> f >> t))
      return false;

    obj = std::make_tuple(f, t, TWeight(1));
    return true;
  };

public:
  graph_istream_edgelist(std::istream& s)
    : graph_istream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
class graph_ostream_edgelist : public graph_ostream<TIndex, TWeight>
{
protected:
  bool
  write_edge(std::ostream& s, std::tuple<TIndex, TIndex, TWeight>& obj) override
  {
    TIndex  f, t;
    TWeight w;

    std::tie(f, t, w) = obj;

    return (bool)(s << f << ' ' << t << '\n');
  };

public:
  graph_ostream_edgelist(std::ostream& s)
    : graph_ostream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
class graph_istream_dimacs : public graph_istream<TIndex, TWeight>
{
protected:
  bool
  read_edge(std::istream& s, std::tuple<TIndex, TIndex, TWeight>& obj) override
  {
    char        c;
    std::string v;

    for (;;) {
      if (!std::getline(s, v))
        return false;

      std::stringstream ss(v);
      if (!(ss >> c))
        return false;

      switch (c) {
        case 'c':
        case 'p':
          continue;
        case 'a': {
          TIndex  f, t;
          TWeight w;
          if (!(ss >> f >> t >> w))
            return false;

          obj = std::make_tuple(f, t, w);
          return true;
        }
        default:
          return false;
      }
    }
  };

public:
  graph_istream_dimacs(std::istream& s)
    : graph_istream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
class graph_istream_weightlist : public graph_istream<TIndex, TWeight>
{
protected:
  bool
  read_edge(std::istream& s, std::tuple<TIndex, TIndex, TWeight>& obj) override
  {
    TIndex  f, t;
    TWeight w;
    if (!(s >> f >> t >> w))
      return false;

    obj = std::make_tuple(f, t, w);
    return true;
  };

public:
  graph_istream_weightlist(std::istream& s)
    : graph_istream<TIndex, TWeight>(s){};
};

template<typename TIndex, typename TWeight>
std::unique_ptr<graph_istream<TIndex, TWeight>>
make_graph_istream(std::istream& s, graph_stream_format format)
{
  switch (format) {
    case graph_stream_format::edgelist:
      return std::unique_ptr<graph_istream<TIndex, TWeight>>(new graph_istream_edgelist<TIndex, TWeight>(s));
    case graph_stream_format::weightlist:
      return std::unique_ptr<graph_istream<TIndex, TWeight>>(new graph_istream_weightlist<TIndex, TWeight>(s));
    case graph_stream_format::dimacs:
      return std::unique_ptr<graph_istream<TIndex, TWeight>>(new graph_istream_dimacs<TIndex, TWeight>(s));
  };
  throw std::logic_error("erro: unknown graph format");
};

template<typename TIndex, typename TWeight>
std::unique_ptr<graph_ostream<TIndex, TWeight>>
make_graph_ostream(std::ostream& s, graph_stream_format format)
{
  switch (format) {
    case graph_stream_format::edgelist:
      return std::unique_ptr<graph_ostream<TIndex, TWeight>>(new graph_ostream_edgelist<TIndex, TWeight>(s));
  };
  throw std::logic_error("erro: unknown graph format");
};

template<typename Graph, typename GraphSetSizeOperation, typename GraphSetValueOperation>
void
scan_graph(std::istream& s, bool binary, Graph& g, GraphSetSizeOperation& set_size, GraphSetValueOperation& set_value)
{
  using size_type  = typename GraphSetSizeOperation::result_type;
  using value_type = typename GraphSetValueOperation::result_type;

  // Read vertex and edge count
  //
  size_type sz = size_type(0);
  if (binary) {
    if (!s.read(reinterpret_cast<char*>(&sz), sizeof(size_type)))
      throw std::logic_error("erro: can't scan adjacency matrix size from binary file");
  } else {
    if (!(s >> sz))
      throw std::logic_error("erro: can't scan adjacency matrix size; expected format: <size>");
  }

  // Resize graph
  //
  set_size(g, sz);

  if (binary) {
    // Read exact number of items from stream
    //
    for (size_type i = size_type(0); i < sz; ++i)
      for (size_type j = size_type(0); j < sz; ++j) {
        value_type v;
        if (!s.read(reinterpret_cast<char*>(&v), sizeof(value_type)))
          throw std::logic_error("erro: can't scan matrix from binary file - the stream ended unexpectadly.");

        set_value(g, i, j, v);
      }
  } else {
    // Keep reading edges (`from vertex` `to vertex` `the weight`)
    // as many as possible
    //
    value_type v;
    size_type  f, t;
    while (s >> f >> t >> v)
      if (v != value_type())
        set_value(g, f, t, v);
  }
};

template<typename Graph, typename GraphGetSizeOperation, typename GraphGetValueOperation>
void
print_graph(std::ostream& s, bool binary, Graph& g, GraphGetSizeOperation& get_size, GraphGetValueOperation& get_value)
{
  using size_type  = typename GraphGetSizeOperation::result_type;
  using value_type = typename GraphGetValueOperation::result_type;

  // Obtain size of the adjacency matrix
  //
  size_type sz = get_size(g);
  if (sz == size_type(0))
    return;

  if (binary) {
    if (!s.write(reinterpret_cast<char*>(&sz), sizeof(size_type)))
      throw std::logic_error("erro: can't print adjacency matrix size");

    for (size_type i = size_type(0); i < sz; ++i)
      for (size_type j = size_type(0); j < sz; ++j) {
        value_type v = get_value(g, i, j);
        if (!s.write(reinterpret_cast<char*>(&v), sizeof(value_type)))
          throw std::logic_error("erro: can't print adjacency matrix cell value");
      }
  } else {
    if (!(s << sz << '\n'))
      throw std::logic_error("erro: can't print adjacency matrix size");

    for (size_type i = size_type(0); i < sz; ++i)
      for (size_type j = size_type(0); j < sz; ++j) {
        value_type v = get_value(g, i, j);
        if (v != value_type())
          if (!(s << i << ' ' << j << ' ' << v << '\n'))
            throw std::logic_error("erro: can't print adjacency matrix cell value");
      }
  }

  s.flush();
};

} // namespace io
} // namespace graphs
} // namespace utilz

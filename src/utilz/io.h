#include <string>

namespace utilz {

template<typename T>
static const std::string
_printf_value_fmt()
{
  static_assert("T does not have correct symbolic representation for 'printf'");
};

template<typename T>
static const std::string
_scanf_value_fmt()
{
  static_assert("T does not have correct symbolic representation for 'scanf'");
};

template<>
inline const std::string
_printf_value_fmt<float>()
{
  return "%.5f000";
};

template<>
inline const std::string
_printf_value_fmt<int>()
{
  return "%d";
};

template<>
inline const std::string
_printf_value_fmt<long>()
{
  return "%ld";
};

template<>
inline const std::string
_printf_value_fmt<long long>()
{
  return "%lld";
};

template<>
inline const std::string
_scanf_value_fmt<float>()
{
  return "%f";
};

template<>
inline const std::string
_scanf_value_fmt<int>()
{
  return "%d";
};

template<>
inline const std::string
_scanf_value_fmt<long>()
{
  return "%ld";
};

template<>
inline const std::string
_scanf_value_fmt<long long>()
{
  return "%lld";
};

template<typename T>
static const std::string
printf_fmt()
{
  return _printf_value_fmt<T>();
};

template<typename T>
static const std::string
scanf_fmt()
{
  return _scanf_value_fmt<T>();
};

template<typename G, typename A>
class graph_output
{
private:
  A& m_allocation_scope;

  G m_graph;

public:
  graph_output(A& allocation_scope)
    : m_allocation_scope(allocation_scope)
  {}

  void prep(size_t vertex_count, size_t edge_count)
  {
    this->m_graph = this->m_allocation_scope.allocate(vertex_count, edge_count);
  }
  void write(size_t i, size_t j, long v)
  {
    this->m_graph(i, j) = v;
  }

  G& operator()()
  {
    return this->m_graph;
  }
};

template<typename T, typename O>
void
scan_graph_from_dimacs9_file(const std::string& path, O& out)
{
  FILE* p_file;
  if (::fopen_s(&p_file, path.c_str(), "r"))
    throw std::runtime_error("erro: can't open a file");

  char buffer[BUFSIZ];
  if (::setvbuf(p_file, buffer, _IOFBF, BUFSIZ))
    throw std::runtime_error("erro: can't initialize a buffer");

  try {
    const std::string error_msg = "erro: bad file format";
    const std::string fmt_s     = "%zu %zu " + utilz::scanf_fmt<T>() + '\n';
    const char*       fmt       = fmt_s.c_str();

    size_t vertex_count, edge_count;
    if (::feof(p_file) || ::fscanf_s(p_file, "%zu\n%zu", &vertex_count, &edge_count) != 2 || ::ferror(p_file))
      throw std::runtime_error("erro: file is incorrect format - can't read vertex and edges count, "
                               "the expected format is <vertext count> <edge count>");

    out.prep(vertex_count, edge_count);

    T      v;
    size_t i, j;
    for (size_t edge = 0; edge < edge_count; ++edge) {
      if (::feof(p_file) || ::fscanf_s(p_file, fmt, &i, &j, &v) != 3 || ::ferror(p_file))
        throw std::runtime_error("erro: file is incorrect format - can't read edge data, "
                                 "the expected format is <vertext> <vertex> <weight>.");

      out.write(i, j, v);
    };
    ::fclose(p_file);
  } catch (...) {
    ::fclose(p_file);
    throw;
  };
};

} // namespace utilz

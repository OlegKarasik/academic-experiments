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

template<typename T, typename O>
void
scan_graph_from_dimacs9_file(const std::string& file_path, O& out)
{
  FILE* p_file;
  if (::fopen_s(&p_file, file_path.c_str(), "r"))
    throw std::runtime_error("erro: can't open a file");

  char buffer[iobuf_sz];
  if (::setvbuf(p_file, buffer, _IOFBF, iobuf_sz))
    throw std::runtime_error("erro: can't initialize a buffer");

  try {
    const std::string error_msg = "erro: bad file format";
    const std::string fmt_s = "%zu %zu " + utilz::scanf_fmt<T>() + '\n';
    const char* fmt = fmt_s.c_str();

    size_t vertex_count, edge_count;
    if (::feof(p_file) || ::fscanf_s(p_file, "%zu\n%zu", &vertex_count, &edge_count) != 2 || ::ferror(p_file))
      throw std::runtime_error("erro: file is incorrect format - can't read vertex and edges count, "
                               "the expected format is <vertext count> <edge count>");

    out.prep(vertex_count, edge_count);

    T v;
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

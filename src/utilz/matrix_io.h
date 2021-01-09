#pragma once

#include "io.h"

namespace utilz {

template<typename T, typename P, typename G>
void
fscan_matrix(const std::string& path, P& predicate, G& matrix_output)
{
  FILE* file;
  if (::fopen_s(&file, path.c_str(), "r"))
    throw std::runtime_error("erro: can't open a file");

  char buffer[BUFSIZ];
  if (::setvbuf(file, buffer, _IOFBF, BUFSIZ))
    throw std::runtime_error("erro: can't initialize a buffer");

  try {
    const std::string fmt_s = "%zu %zu " + utilz::scanf_fmt<T>() + '\n';
    const char*       fmt   = fmt_s.c_str();

    size_t w, h;
    if (::feof(file) || ::fscanf_s(file, "%zu %zu", &w, &h) != 2 || ::ferror(file))
      throw std::runtime_error("erro: file is incorrect format - can't read matrix width and height from a file, "
                               "the expected format is <width> <height>");

    matrix_output.prep(w, h);

    while (!::feof(file)) {
      T      v;
      size_t i, j;
      if (::fscanf_s(file, fmt, &i, &j, &v) != 3 || ::ferror(file))
        throw std::runtime_error("erro: file is incorrect format - can't read cell value from a file, "
                                 "the expected format is <row> <column> <value>.");

      if (predicate(v)) {
        matrix_output(i, j) = v;
      }
    }
    ::fclose(file);
  } catch (...) {
    ::fclose(file);
    throw;
  };
};

template<typename T, typename P, typename S>
void
fprint_matrix(const std::string& path, P& predicate, S& matrix_input)
{
  FILE* file;
  if (::fopen_s(&file, path.c_str(), "w"))
    throw std::runtime_error("erro: can't open a file");

  char buffer[BUFSIZ];
  if (::setvbuf(file, buffer, _IOFBF, BUFSIZ))
    throw std::runtime_error("erro: can't initialize a buffer");

  try {
    const std::string fmts = "\n%zu %zu " + utilz::printf_fmt<T>();
    const char*       fmt  = fmts.c_str();

    if (::fprintf_s(file, "%zu %zu", matrix_input.w(), matrix_input.h()) < 0)
      throw std::runtime_error("erro: can't write matrix width and height to a file.");

    for (size_t i = 0; i < matrix_input.w(); ++i) {
      for (size_t j = 0; j < matrix_input.h(); ++j) {
        T v = matrix_input(i, j);
        if (predicate(v)) {
          if (::fprintf_s(file, fmt, i, j, v) < 0)
            throw std::runtime_error("erro: can't write cell value to a file.");
        }
      }
    }
    ::fflush(file);
    ::fclose(file);
  } catch (...) {
    ::fclose(file);
    ::remove(path.c_str());
    throw;
  };
};


template<typename T>
class matrix_all_predicate
{
public:
  bool operator()(const T& v)
  {
    return true;
  }
};

template<typename T, T V>
class matrix_except_predicate
{
private:
  const T m_v = V;

public:
  matrix_except_predicate()
  {}

  bool operator()(const T& v)
  {
    return v != this->m_v;
  }
};

} // namespace utilz

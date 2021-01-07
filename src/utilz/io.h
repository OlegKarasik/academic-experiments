#pragma once

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

} // namespace utilz

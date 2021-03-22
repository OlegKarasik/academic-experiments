#pragma once

namespace utilz {

class cmdline_arg_config
{
private:
  std::string m_key;
  std::string m_global_key;
  std::string m_description;

  bool m_is_flag;
  bool m_is_required;

public:
  libfn cmdline_arg_config(const std::string& key,
                           const std::string& global_key,
                           const std::string& description,
                           bool               is_flag,
                           bool               is_required);
  libfn cmdline_arg_config(const cmdline_arg_config& other);
  libfn ~cmdline_arg_config();

  const libfn bool is_flag() const;
  const libfn bool is_required() const;

  const libfn std::string key() const;

  const libfn std::string global_key() const;

  const libfn std::string description() const;

  libfn bool operator==(const cmdline_arg_config& other) const;
  libfn bool operator!=(const cmdline_arg_config& other) const;
};

class cmdline_arg
{
private:
  const cmdline_arg_config* const mp_config;
  std::string                     m_value;

public:
  libfn cmdline_arg(const cmdline_arg_config& config);
  libfn cmdline_arg(const cmdline_arg_config& config,
                    const std::string&        value);
  libfn cmdline_arg(const cmdline_arg& other);
  libfn ~cmdline_arg();

  const libfn bool is_flag() const;

  const libfn std::string key() const;

  const libfn std::string global_key() const;

  const libfn std::string value() const;

  libfn bool operator<(const cmdline_arg& other) const;
  libfn bool operator==(const cmdline_arg& other) const;
  libfn bool operator!=(const cmdline_arg& other) const;
};

class cmdline
{
private:
  const std::string m_s_arg = "-";
  const std::string m_g_arg = "--";
  const std::string m_e_arg = "";

  std::map<std::string, cmdline_arg_config> m_s_arg_configs;
  std::map<std::string, cmdline_arg_config> m_g_arg_configs;

  std::vector<cmdline_arg> m_args;

  bool _is_arg(const std::string& value) const;

  const cmdline_arg* _find_arg(const std::string& arg_key,
                               const bool         throw_on) const;

public:
  libfn cmdline(const std::vector<cmdline_arg_config> arg_configs);
  libfn cmdline(const cmdline& other);
  libfn ~cmdline();

  const libfn std::vector<cmdline_arg> args() const;

  const libfn std::string get_arg(const std::string& arg_key) const;

  const libfn std::string get_arg_opt(const std::string& arg_key) const;

  const libfn bool has_arg(const std::string& arg_key) const;

  libfn void parse(size_t args, char** argv);

  libfn bool operator==(const cmdline& other) const;
  libfn bool operator!=(const cmdline& other) const;
}

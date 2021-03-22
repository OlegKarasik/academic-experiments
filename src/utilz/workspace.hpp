#pragma once

#include <filesystem>

namespace workspace {

std::filesystem::path
root()
{
  std::filesystem::path path = std::filesystem::current_path();
  if (std::filesystem::exists(path / ".root"))
    return path;

  while (path.has_parent_path()) {
    path = path.parent_path();
    if (std::filesystem::exists(path / ".root"))
      return path;
  }
  throw std::logic_error("erro: imposible to determin project root; ensure '.root' file exist in the directory path.");
}



} // namespace workspace

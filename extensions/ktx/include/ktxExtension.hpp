#pragma once

#include <filesystem>
#include <string_view>

#include "Image.hpp"

namespace Marbas {

struct KtxContext {
 public:
  virtual Image*
  LoadFromPath(std::filesystem::path& path) = 0;
};

struct KtxExtension {
  using ContextInterface = KtxContext;
  constexpr static std::string_view name = "KtxExtension";

  static std::unique_ptr<ContextInterface>
  CreateVulkanContext();
};

}  // namespace Marbas

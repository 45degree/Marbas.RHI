#pragma once

#include "ktxExtension.hpp"

namespace Marbas {

struct VulkanKtxLoader final : public KtxContext {
 public:
  Image*
  LoadFromPath(std::filesystem::path& path) override;
};

}  // namespace Marbas

#include "ktxExtension.hpp"

#include "VulkanKtxLoader.hpp"

namespace Marbas {

std::unique_ptr<KtxExtension::ContextInterface>
KtxExtension::CreateVulkanContext() {
  return std::make_unique<VulkanKtxLoader>();
}

}  // namespace Marbas

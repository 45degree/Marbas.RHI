#include "RHIFactory.hpp"

#include <glog/logging.h>

#ifdef USE_VULKAN
#include "Vulkan/VulkanRHIFactory.hpp"
#endif

#ifdef USE_D3D12
#include "DirectX12/DirectX12RHIFactory.hpp"
#endif

namespace Marbas {

std::unique_ptr<RHIFactory>
RHIFactory::CreateInstance(const RendererType& rendererTyoe) {
#ifdef USE_VULKAN
  if (rendererTyoe == RendererType::VULKAN) {
    return std::unique_ptr<RHIFactory>(new VulkanRHIFactory());
  }
#endif

#ifdef USE_D3D12
  if (rendererTyoe == RendererType::DirectX12) {
    return std::unique_ptr<RHIFactory>(new DirectX12RHIFactory());
  }
#endif

  DLOG_ASSERT(false);
  return nullptr;
}

}  // namespace Marbas

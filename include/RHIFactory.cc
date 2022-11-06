#include "RHIFactory.hpp"

#include <glog/logging.h>

#ifdef USE_VULKAN
#include "vulkan/VulkanRHIFactory.hpp"
#endif

namespace Marbas {

std::unique_ptr<RHIFactory>
RHIFactory::CreateInstance(const RendererType& rendererTyoe) {
  if (rendererTyoe == RendererType::VULKAN) {
    return std::unique_ptr<RHIFactory>(new VulkanRHIFactory());
  }

  DLOG_ASSERT(false);
  return nullptr;
}

}  // namespace Marbas

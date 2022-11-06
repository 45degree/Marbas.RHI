#include "VulkanPipelineContext.hpp"

namespace Marbas {

Sampler*
VulkanPipelineContext::CreateSampler() {
  // TODO
  vk::SamplerCreateInfo createInfo;
  m_device.createSampler(createInfo);
}

void
VulkanPipelineContext::DestroySampler(Sampler* sampler) {}

DescriptorPool*
VulkanPipelineContext::CreateDescriptorPool() {
  return nullptr;
}

}  // namespace Marbas

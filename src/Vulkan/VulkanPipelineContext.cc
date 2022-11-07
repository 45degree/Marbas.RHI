#include "VulkanPipelineContext.hpp"

#include "VulkanDescriptor.hpp"
#include "vulkanUtil.hpp"

namespace Marbas {

Sampler*
VulkanPipelineContext::CreateSampler() {
  // TODO:
  vk::SamplerCreateInfo createInfo;
  m_device.createSampler(createInfo);

  return nullptr;
}

void
VulkanPipelineContext::DestroySampler(Sampler* sampler) {}

DescriptorPool*
VulkanPipelineContext::CreateDescriptorPool(std::span<DescriptorPoolSize> descritorPoolSize, uint32_t maxSet) {
  vk::DescriptorPoolCreateInfo createInfo;
  createInfo.setMaxSets(maxSet);

  std::vector<vk::DescriptorPoolSize> poolSizes;
  for (auto& descriptorInfo : descritorPoolSize) {
    vk::DescriptorPoolSize poolSize;
    poolSize.setDescriptorCount(descriptorInfo.size);
    poolSize.setType(ConvertToVulkanDescriptorType(descriptorInfo.type));

    poolSizes.push_back(poolSize);
  }

  auto vkDescriptorPool = m_device.createDescriptorPool(createInfo);

  auto* descriptorPool = new VulkanDescriptorPool();
  descriptorPool->vkDescriptorPool = vkDescriptorPool;

  return descriptorPool;
}

void
VulkanPipelineContext::DestroyDescriptorPool(DescriptorPool* descriptorPool) {
  auto* vulkanDescriptorPool = static_cast<VulkanDescriptorPool*>(descriptorPool);

  m_device.destroyDescriptorPool(vulkanDescriptorPool->vkDescriptorPool);

  delete vulkanDescriptorPool;
}

DescriptorSet*
VulkanPipelineContext::CreateDescriptorSet(const DescriptorPool* pool, const DescriptorSetLayout& layout) {
  auto* vulkanDescriptorSet = new VulkanDescriptorSet();

  // create descriptor set layout
  vk::DescriptorSetLayoutCreateInfo layoutCreatInfo;
  std::vector<vk::DescriptorSetLayoutBinding> vkLayoutBinding;
  std::transform(layout.cbegin(), layout.cend(), std::back_inserter(vkLayoutBinding), [](auto&& binding) {
    auto vkBinding = ConvertToVulkanDescriptorLayoutBinding(binding);
    return vkBinding;
  });
  layoutCreatInfo.setBindings(vkLayoutBinding);

  auto vkLayout = m_device.createDescriptorSetLayout(layoutCreatInfo);
  vulkanDescriptorSet->vkLayout = vkLayout;

  vk::DescriptorSetAllocateInfo allocateInfo;
  allocateInfo.setSetLayouts(vkLayout);
  allocateInfo.setDescriptorPool(static_cast<const VulkanDescriptorPool*>(pool)->vkDescriptorPool);
  allocateInfo.setDescriptorSetCount(1);

  auto vkDescriptorSets = m_device.allocateDescriptorSets(allocateInfo);
  DLOG_ASSERT(vkDescriptorSets.size() == 1);

  vulkanDescriptorSet->vkDescriptorSet = vkDescriptorSets[0];

  return vulkanDescriptorSet;
}

void
VulkanPipelineContext::DestroyDescriptorSet(const DescriptorPool* descriptorPool, DescriptorSet* descriptorSet) {
  auto* vulkanDescriptorSet = static_cast<VulkanDescriptorSet*>(descriptorSet);
  const auto* vulkanDescriptorPool = static_cast<const VulkanDescriptorPool*>(descriptorPool);
  auto vkPool = vulkanDescriptorPool->vkDescriptorPool;
  auto vkSet = vulkanDescriptorSet->vkDescriptorSet;
  auto vkLayout = vulkanDescriptorSet->vkLayout;

  m_device.freeDescriptorSets(vkPool, vkSet);
  m_device.destroyDescriptorSetLayout(vkLayout);

  delete vulkanDescriptorSet;
}

/**
 * pipeline
 */

// graphics pipeline
Pipeline*
VulkanPipelineContext::CreatePipeline(GraphicsPipeLineCreateInfo& createInfo) {
  vk::GraphicsPipelineCreateInfo vkCreateInfo;
  return nullptr;
}
}  // namespace Marbas

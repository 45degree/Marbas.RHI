#include "VulkanPipelineContext.hpp"

#include <fstream>
#include <optional>

#include "VulkanDescriptor.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanShaderModule.hpp"
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

ShaderModule*
VulkanPipelineContext::CreateShaderModule(const std::string& spirvPath) {
  auto* vulkanShaderModule = new VulkanShaderModule();

  std::ifstream file(spirvPath, std::ios::binary | std::ios::in);
  std::vector<char> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

  vk::ShaderModuleCreateInfo vkCreateInfo;
  vkCreateInfo.setPCode(reinterpret_cast<uint32_t*>(content.data()));
  vkCreateInfo.setCodeSize(content.size());

  vulkanShaderModule->vkShaderModule = m_device.createShaderModule(vkCreateInfo);

  return vulkanShaderModule;
}

void
VulkanPipelineContext::DestroyShaderModule(ShaderModule* shaderModule) {
  auto* vulkanShaderModule = static_cast<VulkanShaderModule*>(shaderModule);
  m_device.destroyShaderModule(vulkanShaderModule->vkShaderModule);

  delete vulkanShaderModule;
}

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
  auto* graphicsPipeline = new VulkanPipeline();
  graphicsPipeline->vkRenderPass = CreateRenderPass(createInfo.outputRenderTarget);

  vk::GraphicsPipelineCreateInfo vkCreateInfo;
  vkCreateInfo.setRenderPass(graphicsPipeline->vkRenderPass);

  // input state
  vk::PipelineVertexInputStateCreateInfo vkVertexInputStateCreateInfo;
  if (!createInfo.vertexInputLayout.elementDesc.empty()) {
    std::vector<vk::VertexInputAttributeDescription> vkInputAttributeDescs;
    std::vector<vk::VertexInputBindingDescription> vkInputBindingDescs;
    for (auto layout : createInfo.vertexInputLayout.elementDesc) {
      vk::VertexInputAttributeDescription vkInputAttributeDesc;
      vkInputAttributeDesc.setBinding(layout.binding);
      vkInputAttributeDesc.setLocation(layout.attribute);
      vkInputAttributeDesc.setOffset(layout.offset);
      vkInputAttributeDesc.setFormat(ConvertToVulkanFormat(layout.format));
      vkInputAttributeDescs.push_back(vkInputAttributeDesc);
    }
    for (auto viewDesc : createInfo.vertexInputLayout.viewDesc) {
      vk::VertexInputBindingDescription vkInputBindingDesc;
      vkInputBindingDesc.setBinding(viewDesc.binding);
      vkInputBindingDesc.setStride(viewDesc.stride);
      if (viewDesc.inputClass == VertexInputClass::VERTEX) {
        vkInputBindingDesc.setInputRate(vk::VertexInputRate::eVertex);
      } else {
        vkInputBindingDesc.setInputRate(vk::VertexInputRate::eInstance);
      }

      vkInputBindingDescs.push_back(vkInputBindingDesc);
    }
    vkVertexInputStateCreateInfo.setVertexAttributeDescriptions(vkInputAttributeDescs);
    vkVertexInputStateCreateInfo.setVertexBindingDescriptions(vkInputBindingDescs);
  }

  // input assembly state

  // shader stage
  vk::PipelineShaderStageCreateInfo vkShaderStageCreateInfo;

  // view port

  // resterization state
  vk::PipelineRasterizationStateCreateInfo vkRasterizationStateCreateInfo;

  // depth stecil

  // blend

  // sample

  // auto result = m_device.createGraphicsPipeline(nullptr, vkCreateInfo);
  // if (result.result != vk::Result::eSuccess) {
  //   throw std::runtime_error("failed to create the vulkan graphics pipepine");
  // }
  // graphicsPipeline->vkPipeline = result.value;
  return graphicsPipeline;
}

vk::RenderPass
VulkanPipelineContext::CreateRenderPass(const std::vector<RenderTargetDesc>& renderTargetDescs) {
  std::vector<vk::AttachmentDescription> attachmentDescriptions;
  std::vector<vk::AttachmentReference> colorAttachmentReferences;
  std::optional<vk::AttachmentReference> depthAttachmentReference;

  for (auto renderTargetDesc : renderTargetDescs) {
    vk::AttachmentDescription description;
    vk::AttachmentReference reference;
    reference.setAttachment(attachmentDescriptions.size());

    if (!renderTargetDesc.isDepth) {
      description.setInitialLayout(vk::ImageLayout::eUndefined);
      description.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
      reference.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
      colorAttachmentReferences.push_back(reference);
    } else {
      description.setInitialLayout(vk::ImageLayout::eGeneral);
      description.setFinalLayout(vk::ImageLayout::eDepthAttachmentOptimal);
      reference.setLayout(vk::ImageLayout::eDepthAttachmentOptimal);
      depthAttachmentReference = reference;
    }
    description.setFormat(ConvertToVulkanFormat(renderTargetDesc.format));

    // TODO:
    description.setSamples(vk::SampleCountFlagBits::e1);

    if (!renderTargetDesc.isClear) {
      description.setLoadOp(vk::AttachmentLoadOp::eClear);
    } else {
      description.setLoadOp(vk::AttachmentLoadOp::eDontCare);
    }
    description.setStoreOp(vk::AttachmentStoreOp::eStore);

    attachmentDescriptions.push_back(description);
  }

  vk::SubpassDescription vkSubpassDescription;
  vkSubpassDescription.setColorAttachments(colorAttachmentReferences);

  if (depthAttachmentReference.has_value()) {
    vkSubpassDescription.setPDepthStencilAttachment(&depthAttachmentReference.value());
  }

  vk::RenderPassCreateInfo vkRenderPassCreateInfo;
  vkRenderPassCreateInfo.setAttachments(attachmentDescriptions);
  vkRenderPassCreateInfo.setSubpasses(vkSubpassDescription);

  return m_device.createRenderPass(vkRenderPassCreateInfo);
}

}  // namespace Marbas

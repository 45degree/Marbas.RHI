#include "VulkanPipelineContext.hpp"

#include <fstream>
#include <optional>

#include "VulkanDescriptor.hpp"
#include "VulkanImage.hpp"
#include "VulkanPipeline.hpp"
#include "VulkanShaderModule.hpp"
#include "vulkanUtil.hpp"

namespace Marbas {

Sampler*
VulkanPipelineContext::CreateSampler() {
  // TODO:
  vk::SamplerCreateInfo createInfo;
  auto sampler = m_device.createSampler(createInfo);

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

  // render pass
  vk::GraphicsPipelineCreateInfo vkCreateInfo;
  vkCreateInfo.setRenderPass(graphicsPipeline->vkRenderPass);

  // input state
  vk::PipelineVertexInputStateCreateInfo vkVertexInputStateCreateInfo;
  std::vector<vk::VertexInputAttributeDescription> vkInputAttributeDescs;
  std::vector<vk::VertexInputBindingDescription> vkInputBindingDescs;
  if (!createInfo.vertexInputLayout.elementDesc.empty()) {
    for (const auto& layout : createInfo.vertexInputLayout.elementDesc) {
      vk::VertexInputAttributeDescription vkInputAttributeDesc;
      vkInputAttributeDesc.setBinding(layout.binding);
      vkInputAttributeDesc.setLocation(layout.attribute);
      vkInputAttributeDesc.setOffset(layout.offset);
      vkInputAttributeDesc.setFormat(ConvertToVulkanFormat(layout.format));
      vkInputAttributeDescs.push_back(vkInputAttributeDesc);
    }
    for (const auto& viewDesc : createInfo.vertexInputLayout.viewDesc) {
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
  vkCreateInfo.setPVertexInputState(&vkVertexInputStateCreateInfo);

  // input assembly state
  vk::PipelineInputAssemblyStateCreateInfo vkInputAssemblyStateCreateInfo;
  vkInputAssemblyStateCreateInfo.setPrimitiveRestartEnable(false);
  vkInputAssemblyStateCreateInfo.setTopology(ConvertToVulkanTopology(createInfo.inputAssemblyState.topology));
  vkCreateInfo.setPInputAssemblyState(&vkInputAssemblyStateCreateInfo);

  // shader stage
  std::vector<vk::PipelineShaderStageCreateInfo> vkShaderStageCreateInfos;
  for (auto& shaderStageCreateInfo : createInfo.shaderStageCreateInfo) {
    vk::PipelineShaderStageCreateInfo info;
    auto module = static_cast<VulkanShaderModule*>(shaderStageCreateInfo.shaderModule);
    info.setModule(module->vkShaderModule);
    if (shaderStageCreateInfo.stage == ShaderType::VERTEX_SHADER) {
      info.setStage(vk::ShaderStageFlagBits::eVertex);
    } else if (shaderStageCreateInfo.stage == ShaderType::FRAGMENT_SHADER) {
      info.setStage(vk::ShaderStageFlagBits::eFragment);
    } else if (shaderStageCreateInfo.stage == ShaderType::GEOMETRY_SHADER) {
      info.setStage(vk::ShaderStageFlagBits::eGeometry);
    }
    info.setPName(shaderStageCreateInfo.interName.c_str());
    vkShaderStageCreateInfos.push_back(info);
  }
  vkCreateInfo.setStages(vkShaderStageCreateInfos);

  // view port
  vk::PipelineViewportStateCreateInfo vkViewportStateCreateInfo;
  vkCreateInfo.setPViewportState(&vkViewportStateCreateInfo);

  // dynamic state
  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo vkPipelineDynamicStateCreateInfo;
  vkPipelineDynamicStateCreateInfo.setDynamicStates(dynamicStates);
  vkCreateInfo.setPDynamicState(&vkPipelineDynamicStateCreateInfo);

  // resterization state
  vk::PipelineRasterizationStateCreateInfo vkRasterizationStateCreateInfo;
  vkRasterizationStateCreateInfo.setRasterizerDiscardEnable(false);
  vkRasterizationStateCreateInfo.setCullMode(ConvertToVulkanCullMode(createInfo.rasterizationInfo.cullMode));
  vkRasterizationStateCreateInfo.setLineWidth(1.0);
  vkRasterizationStateCreateInfo.setPolygonMode(ConvertToVulkanPolyMode(createInfo.rasterizationInfo.polygonMode));
  vkRasterizationStateCreateInfo.setFrontFace(ConvertToVulkanFrontFace(createInfo.rasterizationInfo.frontFace));

  vkRasterizationStateCreateInfo.setDepthClampEnable(true);
  vkRasterizationStateCreateInfo.setDepthBiasClamp(createInfo.rasterizationInfo.depthBiasClamp);
  vkRasterizationStateCreateInfo.setDepthBiasConstantFactor(0);
  vkRasterizationStateCreateInfo.setDepthBiasSlopeFactor(createInfo.rasterizationInfo.slopeScaledDepthBias);
  vkRasterizationStateCreateInfo.setDepthBiasEnable(false);

  vk::PipelineRasterizationDepthClipStateCreateInfoEXT vkDepthClipStateCreateInfo;
  vkDepthClipStateCreateInfo.setDepthClipEnable(createInfo.rasterizationInfo.depthCilpEnable);
  vkRasterizationStateCreateInfo.setPNext(&vkDepthClipStateCreateInfo);

  vkCreateInfo.setPRasterizationState(&vkRasterizationStateCreateInfo);

  // depth stecil
  vk::PipelineDepthStencilStateCreateInfo vkDepthStencilStateCreateInfo;
  vkDepthStencilStateCreateInfo.setDepthTestEnable(createInfo.depthStencilInfo.depthTestEnable);
  vkDepthStencilStateCreateInfo.setDepthWriteEnable(createInfo.depthStencilInfo.depthWriteEnable);
  vkDepthStencilStateCreateInfo.setDepthBoundsTestEnable(createInfo.depthStencilInfo.depthBoundsTestEnable);
  vkDepthStencilStateCreateInfo.setStencilTestEnable(createInfo.depthStencilInfo.stencilTestEnable);
  switch (createInfo.depthStencilInfo.depthCompareOp) {
    case DepthCompareOp::ALWAYS:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eAlways);
      break;
    case DepthCompareOp::NEVER:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eNever);
      break;
    case DepthCompareOp::LESS:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eLess);
      break;
    case DepthCompareOp::EQUAL:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eEqual);
      break;
    case DepthCompareOp::LEQUAL:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eLessOrEqual);
      break;
    case DepthCompareOp::GREATER:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eGreater);
      break;
    case DepthCompareOp::NOTEQUAL:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eNotEqual);
      break;
    case DepthCompareOp::GEQUAL:
      vkDepthStencilStateCreateInfo.setDepthCompareOp(vk::CompareOp::eGreaterOrEqual);
      break;
  }
  vkCreateInfo.setPDepthStencilState(&vkDepthStencilStateCreateInfo);

  // blend
  vk::PipelineColorBlendStateCreateInfo vkColorBlendStateCreateInfo;
  std::vector<vk::PipelineColorBlendAttachmentState> vkColorBlendAttachmentStates;
  vkColorBlendStateCreateInfo.setBlendConstants(createInfo.blendInfo.constances);
  for (auto& attachment : createInfo.blendInfo.attachments) {
    auto convertToVukanBlendOp = [](const BlendOp& blendOp) -> vk::BlendOp {
      if (blendOp == BlendOp::ADD) return vk::BlendOp::eAdd;
      if (blendOp == BlendOp::REVERSE_SUBTRACT) return vk::BlendOp::eReverseSubtract;
      if (blendOp == BlendOp::SUBTRACT) return vk::BlendOp::eSubtract;

      return vk::BlendOp::eAdd;
    };

    auto convertToVulkanFactory = [](const BlendFactor& factory) -> vk::BlendFactor {
      switch (factory) {
        case BlendFactor::ZERO:
          return vk::BlendFactor::eZero;
        case BlendFactor::ONE:
          return vk::BlendFactor::eOne;
        case BlendFactor::SRC_COLOR:
          return vk::BlendFactor::eSrcColor;
        case BlendFactor::ONE_MINUS_SRC_COLOR:
          return vk::BlendFactor::eOneMinusSrcColor;
        case BlendFactor::DST_COLOR:
          return vk::BlendFactor::eDstColor;
        case BlendFactor::ONE_MINUS_DST_COLOR:
          return vk::BlendFactor::eOneMinusDstColor;
        case BlendFactor::SRC_ALPHA:
          return vk::BlendFactor::eSrcAlpha;
        case BlendFactor::ONE_MINUS_SRC_ALPHA:
          return vk::BlendFactor::eOneMinusSrcAlpha;
        case BlendFactor::DST_ALPHA:
          return vk::BlendFactor::eDstAlpha;
        case BlendFactor::ONE_MINUS_DST_ALPHA:
          return vk::BlendFactor::eOneMinusDstAlpha;
        case BlendFactor::CONSTANT_COLOR:
          return vk::BlendFactor::eConstantColor;
        case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
          return vk::BlendFactor::eOneMinusConstantColor;
        case BlendFactor::CONSTANT_ALPHA:
          return vk::BlendFactor::eConstantAlpha;
        case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
          return vk::BlendFactor::eOneMinusConstantAlpha;
      }
    };

    auto writeMask = vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG |
                     vk::ColorComponentFlagBits::eR;
    vk::PipelineColorBlendAttachmentState attachmentState;
    attachmentState.setBlendEnable(attachment.blendEnable);
    attachmentState.setColorBlendOp(convertToVukanBlendOp(attachment.colorBlendOp));
    attachmentState.setAlphaBlendOp(convertToVukanBlendOp(attachment.alphaBlendOp));
    attachmentState.setSrcColorBlendFactor(convertToVulkanFactory(attachment.srcColorBlendFactor));
    attachmentState.setSrcAlphaBlendFactor(convertToVulkanFactory(attachment.srcAlphaBlendFactor));
    attachmentState.setDstColorBlendFactor(convertToVulkanFactory(attachment.dstColorBlendFactor));
    attachmentState.setDstAlphaBlendFactor(convertToVulkanFactory(attachment.dstAlphaBlendFactor));
    attachmentState.setColorWriteMask(writeMask);

    vkColorBlendAttachmentStates.push_back(attachmentState);
  }
  vkColorBlendStateCreateInfo.setAttachments(vkColorBlendAttachmentStates);
  // TODO: does directx12 supoort it?
  vkColorBlendStateCreateInfo.setLogicOpEnable(false);
  vkCreateInfo.setPColorBlendState(&vkColorBlendStateCreateInfo);

  // sample
  vk::PipelineMultisampleStateCreateInfo vkMultisampleStateCreateInfo;
  const auto& sampleCount = createInfo.multisampleCreateInfo.rasterizationSamples;
  vkMultisampleStateCreateInfo.setSampleShadingEnable(false);
  vkMultisampleStateCreateInfo.setRasterizationSamples(ConvertToVulkanSampleCount(sampleCount));
  vkCreateInfo.setPMultisampleState(&vkMultisampleStateCreateInfo);

  // layout
  auto vkPipelineLayout = CreatePipelineLayout(createInfo.descriptorSetLayout);
  vkCreateInfo.setLayout(vkPipelineLayout);

  auto result = m_device.createGraphicsPipeline(nullptr, vkCreateInfo);
  if (result.result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create the vulkan graphics pipepine");
  }
  graphicsPipeline->vkPipeline = result.value;
  graphicsPipeline->pipelineType = PipelineType::GRAPHICS;
  graphicsPipeline->vkPipelineLayout = vkPipelineLayout;
  return graphicsPipeline;
}

void
VulkanPipelineContext::DestroyPipeline(Pipeline* pipeline) {
  auto* vulkanPipeline = static_cast<VulkanPipeline*>(pipeline);
  m_device.destroyRenderPass(vulkanPipeline->vkRenderPass);
  m_device.destroyPipeline(vulkanPipeline->vkPipeline);
  m_device.destroyPipelineLayout(vulkanPipeline->vkPipelineLayout);
  m_device.destroyDescriptorSetLayout(vulkanPipeline->vkDescriptorSetLayout);

  delete vulkanPipeline;
}

FrameBuffer*
VulkanPipelineContext::CreateFrameBuffer(const FrameBufferCreateInfo& createInfo) {
  auto* vulkanPipeline = static_cast<VulkanPipeline*>(createInfo.pieline);
  auto vkRenderPass = vulkanPipeline->vkRenderPass;

  vk::FramebufferCreateInfo vkFramebufferCreateInfo;
  std::vector<vk::ImageView> vkAttachments;
  std::transform(createInfo.attachments.begin(), createInfo.attachments.end(), std::back_inserter(vkAttachments),
                 [](auto&& attachment) { return static_cast<VulkanImageView*>(attachment)->imageView; });

  vkFramebufferCreateInfo.setRenderPass(vkRenderPass);
  vkFramebufferCreateInfo.setAttachments(vkAttachments);
  vkFramebufferCreateInfo.setLayers(createInfo.layer);
  vkFramebufferCreateInfo.setHeight(createInfo.height);
  vkFramebufferCreateInfo.setWidth(createInfo.width);

  auto vkFramebuffer = m_device.createFramebuffer(vkFramebufferCreateInfo);

  auto* vulkanFrameBuffer = new VulkanFrameBuffer();
  vulkanFrameBuffer->vkFrameBuffer = vkFramebuffer;
  vulkanFrameBuffer->width = createInfo.width;
  vulkanFrameBuffer->height = createInfo.height;
  vulkanFrameBuffer->layerCount = createInfo.layer;

  return vulkanFrameBuffer;
}

void
VulkanPipelineContext::DestroyFrameBuffer(FrameBuffer* frameBuffer) {
  auto* vulkanFrameBuffer = static_cast<VulkanFrameBuffer*>(frameBuffer);
  m_device.destroyFramebuffer(vulkanFrameBuffer->vkFrameBuffer);

  delete vulkanFrameBuffer;
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

    if (renderTargetDesc.isClear) {
      description.setLoadOp(vk::AttachmentLoadOp::eClear);
    } else {
      description.setLoadOp(vk::AttachmentLoadOp::eDontCare);
    }
    description.setStoreOp(vk::AttachmentStoreOp::eStore);

    if (renderTargetDesc.isPresent && !renderTargetDesc.isDepth) {
      description.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
    } else if (!renderTargetDesc.isPresent && !renderTargetDesc.isDepth) {
      description.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
    }

    attachmentDescriptions.push_back(description);
  }

  vk::SubpassDescription vkSubpassDescription;
  vkSubpassDescription.setColorAttachments(colorAttachmentReferences);
  // TODO
  vkSubpassDescription.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

  if (depthAttachmentReference.has_value()) {
    vkSubpassDescription.setPDepthStencilAttachment(&depthAttachmentReference.value());
  }

  vk::RenderPassCreateInfo vkRenderPassCreateInfo;
  vkRenderPassCreateInfo.setAttachments(attachmentDescriptions);
  vkRenderPassCreateInfo.setSubpasses(vkSubpassDescription);

  return m_device.createRenderPass(vkRenderPassCreateInfo);
}

vk::PipelineLayout
VulkanPipelineContext::CreatePipelineLayout(const DescriptorSetLayout& descriptorSetLayout) {
  // TODO:
  vk::PipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
  return m_device.createPipelineLayout(vkPipelineLayoutCreateInfo);
}

}  // namespace Marbas
